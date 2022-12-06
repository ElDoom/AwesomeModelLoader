#include <GL/glew.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp> //for matrix transformation functions

#include "glslprogram.h"
#include "TextureManager.h"
#include "Mesh.h"
#include "Config.h"
#include "Joystick.h"

#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>
#include "FileBrowser/ImGuiFileBrowser.h"



//this macro helps calculate offsets for VBO stuff
//Pass i as the number of bytes for the offset, so be sure to use sizeof() 
//to help calculate bytes accurately.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

//lighting
struct Light
{
	glm::vec3 position;
	glm::vec3 colour;
	float power;
};
string test = "skull";
Mesh *mesh = NULL;

GLFWwindow* window = NULL;

Configuration configuration;

Joystick joystick;

glm::vec3 cameraPosition;
float cameraPitch = 0, cameraYaw = 0;
glm::vec4 cameraDir;
float scalingMovement = 6.f;
float scalingRotation = 2.f;

imgui_addons::ImGuiFileBrowser file_dialog;
string mapLoad;


// from https://www.3dgep.com/understanding-the-view-matrix/
// Pitch should be in the range of [-90 ... 90] degrees and yaw
// should be in the range of [0 ... 360] degrees.
glm::mat4 FPSViewRH(glm::vec3 eye, float pitch, float yaw)
{
	float cosPitch = cos(pitch);
	float sinPitch = sin(pitch);
	float cosYaw = cos(yaw);
	float sinYaw = sin(yaw);

	glm::vec3 xaxis = glm::vec3(cosYaw, 0, -sinYaw);
	glm::vec3 yaxis = glm::vec3(sinYaw * sinPitch, cosPitch, cosYaw * sinPitch);
	glm::vec3 zaxis = glm::vec3(sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw);

	// Create a 4x4 view matrix from the right, up, forward and eye position vectors
	glm::mat4 viewMatrix = glm::mat4(
		glm::vec4(xaxis.x, yaxis.x, zaxis.x, 0),
		glm::vec4(xaxis.y, yaxis.y, zaxis.y, 0),
		glm::vec4(xaxis.z, yaxis.z, zaxis.z, 0),
		glm::vec4(-glm::dot(xaxis, eye), -glm::dot(yaxis, eye), -glm::dot(zaxis, eye), 1)
	);

	return viewMatrix;
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
static void showMainMenu()
{
    bool open = false, save = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Open", NULL))
                open = true;
            if (ImGui::MenuItem("Save", NULL))
                save = true;

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    //Remember the name to ImGui::OpenPopup() and showFileDialog() must be same...
    if (open)
        ImGui::OpenPopup("Open File");
    if (save)
        ImGui::OpenPopup("Save File");

    /* Optional third parameter. Support opening only compressed rar/zip files.
     * Opening any other file will show error, return false and won't close the dialog.
     */
    if (file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(600, 600), ".s3d"))
    {
        std::cout << file_dialog.selected_fn << std::endl;      // The name of the selected file or directory in case of Select Directory dialog mode
        std::cout << file_dialog.selected_path << std::endl;    // The absolute path to the selected file
        std::cout << file_dialog.selected_fn << std::endl;
        mapLoad = file_dialog.selected_fn;

    }
    if (file_dialog.showFileDialog("Save File", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(600, 600), ".s3d"))
    {
        std::cout << file_dialog.selected_fn << std::endl;      // The name of the selected file or directory in case of Select Directory dialog mode
        std::cout << file_dialog.selected_path << std::endl;    // The absolute path to the selected file
        std::cout << file_dialog.ext << std::endl;              // Access ext separately (For SAVE mode)
        //Do writing of files based on extension here
    }
    
}


int main(void)
{
    
    
	if (!LoadConfig(configuration))
	{
		std::cout << "ERROR reading the config file!\n\n";
	}
	else
		DisplayConfig(configuration);

	glfwSetErrorCallback(error_callback);

	if(!glfwInit())
		exit(EXIT_FAILURE);

	window = glfwCreateWindow(1920, 1080, "Context example", NULL, NULL);
	if(!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	const char* glsl_version = "#version 130"; //done here, in case you decide to write code to handle OpenGL ES etc.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwMakeContextCurrent(window);// now we have an OpenGL context for this thread.

	//use GLEW to initialiaze modern opengl functionality supported by the GPU drivers
	glewInit();

	//joystick stuff
	joystick.Init();

	//Make Texture Manager
	TextureManager texManager;

	//load shaders
	GLSLProgram shaders;
	shaders.compileShaderFromFile("vertex.glsl", GLSLShader::GLSLShaderType::VERTEX);
	shaders.compileShaderFromFile("fragment.glsl", GLSLShader::GLSLShaderType::FRAGMENT);

	//bind attributes for the shader layout BEFORE linking the shaders!
	//We only need to do this if we can't use the layout command from within the shader code.
	shaders.bindAttribLocation(0, "position");
	shaders.bindAttribLocation(1, "normal");
	shaders.bindAttribLocation(2, "texUV");

	shaders.link();
	shaders.use();

	//MESH LOAD
    //mesh = new Mesh(&texManager, &shaders, "skull.s3d");
	mesh = new Mesh(&texManager, &shaders, mapLoad);
    std::cout << mapLoad;

	glfwSetKeyCallback(window, key_callback);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//turn on single-sided polygons
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	//view matrix to transform the camera
	glm::mat4 viewMatrix;
	//projection matrix to project the scene onto the monitor
	glm::mat4 projectionMatrix;

	//...and an accumulator for rotatation:
	float angle = 0.f;

	//timer vars
	double currentTime = glfwGetTime();
	double previousTime = currentTime;
	float timePassed;

	cameraPosition = glm::vec3(0, 0, 0);

	//location of the mesh
	glm::vec3 meshPosition(0.0f, 0.0f, 20.f);
	mesh->position = meshPosition;

	//lights
	std::vector<Light> lights;
	Light light1;
	
	//light #1
	light1.position = glm::vec3(-5, -5, 0);
	light1.colour = glm::vec3(0.1f, 0.2f, 0.5f);
	light1.power = 20.f;
	lights.push_back(light1);

	//light #2
	light1.position = glm::vec3(5, 5, 5);
	light1.colour = glm::vec3(0.9f, 0.8f, 0.1f);
	light1.power = 150.f;
	lights.push_back(light1);

	//light #3
	light1.position = glm::vec3(-7, -5, 0);
	light1.colour = glm::vec3(0.7f, 0.2f, 0.5f);
	light1.power = 40.f;
	lights.push_back(light1);

	//light #4
	light1.position = glm::vec3(7, 5, 5);
	light1.colour = glm::vec3(0.2f, 0.8f, 0.1f);
	light1.power = 110.f;
	lights.push_back(light1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);



	while(!glfwWindowShouldClose(window))
	{
        mesh = new Mesh(&texManager, &shaders, mapLoad);
      
		currentTime = glfwGetTime();
		timePassed = static_cast<float>(currentTime - previousTime);
		if (timePassed > 0) previousTime = currentTime;

		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ImGui::Begin("3D Assignment");
        showMainMenu();
		
			//glfwSetWindowShouldClose(window, GL_TRUE);
		//ImGui::SameLine();
		//ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//update projection (needed if window size can be modified)
		projectionMatrix = glm::perspective(45.0f, ratio, 0.1f, 10000.f);

		//send the matrix to the shader
		shaders.setUniform("projectionMatrix", projectionMatrix);

		joystick.DoJoystick(); //handle joystick polling
							   //process joystick values
		joystick.ProcessJoystickAxis(joystick.joystickPositionAxis1, configuration);
		joystick.ProcessJoystickAxis(joystick.joystickPositionAxis2, configuration);
		joystick.ProcessJoystickAxis(joystick.joystickPositionAxis4, configuration);
		joystick.ProcessJoystickAxis(joystick.joystickPositionAxis3, configuration);


		cameraYaw += joystick.joystickPositionAxis3 * scalingRotation * timePassed * configuration.axis_yaw;
		cameraPitch += joystick.joystickPositionAxis4 * scalingRotation * timePassed * configuration.axis_pitch;

		//move ahead
		cameraDir = glm::vec4(0, 0, -1, 0);
		viewMatrix = glm::mat4(1.f);
		viewMatrix = glm::rotate(viewMatrix, cameraYaw, glm::vec3(0, 1.f, 0));
		viewMatrix = glm::rotate(viewMatrix, cameraPitch, glm::vec3(1.f, 0, 0));
		cameraDir = viewMatrix * cameraDir;

		cameraPosition -= glm::vec3(cameraDir) * scalingMovement * joystick.joystickPositionAxis2 * (float)timePassed * configuration.axis_move;

		//strafe
		cameraDir = glm::vec4(1, 0, 0, 0);
		viewMatrix = glm::mat4(1.f);
		//viewMatrix = glm::rotate(viewMatrix, cameraPitch, glm::vec3(1.f, 0, 0));
		viewMatrix = glm::rotate(viewMatrix, cameraYaw, glm::vec3(0, 1.f, 0));
		cameraDir = viewMatrix * cameraDir;

		cameraPosition += glm::vec3(cameraDir) * scalingMovement * joystick.joystickPositionAxis1 * (float)timePassed * configuration.axis_strafe;


		//update camera position via the view matrix
		viewMatrix = FPSViewRH(cameraPosition, cameraPitch, cameraYaw);

		//send the matrix to the shader
		shaders.setUniform("viewMatrix", viewMatrix);

		//update angle
		angle += timePassed * 1;
		mesh->angle = angle;
		mesh->Update(timePassed);

		//lighting
		shaders.setUniform("numLights", (int)lights.size());
		for (int i = 0; i < lights.size(); ++i)
		{
			std::stringstream ss;
			ss << "lightArray[" << i << "].";
			std:string lightString;

			//light position	
			lightString = ss.str() + "position";
			shaders.setUniform(lightString.c_str(), lights[i].position);
			//light colour
			lightString = ss.str() + "colour";
			shaders.setUniform(lightString.c_str(), lights[i].colour);
			//light power
			lightString = ss.str() + "power";
			shaders.setUniform(lightString.c_str(), lights[i].power);
		}
		//material shinyness
		shaders.setUniform("shinyness", 0.5f);
		//material specular "power" : increasing this focuses the specular reflection size
		shaders.setUniform("specularPower", 20.f);
		
		mesh->Draw();

		glfwSwapBuffers(window); //display the graphics buffer to the screen
		glfwPollEvents(); //prime the message pump that GLFW uses for input events
	}



	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	delete mesh;

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}