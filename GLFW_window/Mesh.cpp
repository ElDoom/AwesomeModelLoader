#include "Mesh.h"
#include <fstream>
#include <iostream>

// MESH LOADER
void Mesh::LoadMeshFromFile(std::string filename)
{
	std::ifstream myfile(filename);
	if (myfile.is_open())
	{
		myfile >> numberOfVertices;
		GLfloat tempfloat;
		for (int index = 0; index < numberOfVertices * 8; index++)
		{
			myfile >> tempfloat;
			Vertices.push_back(tempfloat);
		}
		myfile >> numberOfIndices;
		GLushort tempushort;
		for (int index = 0; index < numberOfIndices; index++)
		{
			myfile >> tempushort;
			Indices.push_back(tempushort);
		}
		std::string tempstring;
		myfile >> tempstring;	// assumes the texture filename does not have spaces in its name
		textureFilename = tempstring;
	}
	myfile.close();
}
// CONSTRUCTOR
Mesh::Mesh(TextureManager* tManager, GLSLProgram* shaders, std::string fileName)
{
	LoadMeshFromFile(fileName);
	this->shaders = shaders;
	txManager = tManager;
	texID = txManager->LoadTexture(this->textureFilename);

	angle = 0;
	axis = glm::vec3(0, 1, 0);

	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vboID);

	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vboID);

	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, (Vertices.size() * sizeof(GLfloat)), Vertices.data(), GL_STATIC_DRAW);

	// x,y,z position attrib
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		8 * sizeof(GLfloat),  // stride
		(void*)0            // array buffer offset
	);

	// nx, ny, nz normalized vector attrib
	glVertexAttribPointer(
		1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		8 * sizeof(GLfloat),  // stride
		BUFFER_OFFSET(sizeof(GLfloat) * 3)	// array buffer offset
	);

	// u,v  texture attrib
	glVertexAttribPointer(
		2,                  // attribute 2. No particular reason for 2, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		8 * sizeof(GLfloat),  // stride
		BUFFER_OFFSET(sizeof(GLfloat) * 6)	// array buffer offset
	);

	glEnableVertexAttribArray(0); // channel 0
	glEnableVertexAttribArray(1); // channel 1
	glEnableVertexAttribArray(2); // channel 2

	// Generate 1 buffer, put the resulting identifier in indexbuffer
	glGenBuffers(1, &eboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(GLushort), Indices.data(), GL_STATIC_DRAW);

	// VAO
	glBindVertexArray(0); // Disable our Vertex Array Object? 

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);// Disable our Vertex Buffer Object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// Disable our Element Buffer Object

	return;
}

void Mesh::Draw()
{
	shaders->setUniform("modelMatrix", modelMatrix);			// send the modelMatrix to the shader

	glBindVertexArray(vaoID);

	// bind the texture
	txManager->BindTexture(texID);

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_SHORT, NULL);
	glBindVertexArray(0);
}

void Mesh::Update(double seconds)
{
	// calculate model matrix 
	modelMatrix = glm::mat4(1.f); // identity matrix
								  // note the order of operations here: translate THEN rotate!
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::rotate(modelMatrix, angle, axis);
}

//DESTRUCTOR
Mesh::~Mesh()
{
	glDeleteBuffers(1, &vboID);
	glDeleteBuffers(1, &eboID);
	glDeleteVertexArrays(1, &vaoID);
}