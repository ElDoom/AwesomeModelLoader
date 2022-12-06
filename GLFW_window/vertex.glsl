#version 400
#define MAX_LIGHTS 10

in vec3 position;
in vec3 normal;
in vec2 texUV;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float shinyness;
uniform float specularPower;

out vec3 out_normal;
out vec2 textureUV;

out vec3 eyeDirectionCameraSpace;
out vec3 vertexPositionCameraSpace;
out vec3 LightPosCameraSpace[MAX_LIGHTS];
uniform int numLights;

struct Light
{
	vec3 position;
	vec3 colour;
	float power;
};

uniform Light lightArray[MAX_LIGHTS];

void main(void)
{
	vec4 vertexModelSpace = vec4(position, 1.0);

	//transform the vertices
	vertexPositionCameraSpace = (viewMatrix * modelMatrix * vertexModelSpace).xyz;
	eyeDirectionCameraSpace = vec3(0,0,0) - vertexPositionCameraSpace;

	for(int i = 0; i < numLights; ++i)
	{
		LightPosCameraSpace[i] = (viewMatrix * vec4(lightArray[i].position, 1)).xyz;
	}

	textureUV = texUV;
	out_normal = (viewMatrix *modelMatrix * vec4(normal, 0)).xyz;

	vertexModelSpace =  projectionMatrix * viewMatrix * modelMatrix * vertexModelSpace;
	gl_Position = vertexModelSpace;
} 