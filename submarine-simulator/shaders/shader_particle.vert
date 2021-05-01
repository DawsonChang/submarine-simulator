#version 430 core

layout(location = 2) in vec3 vertexPosition;
layout(location = 3) in vec2 vertexTexCoord;
layout(location = 4) in vec3 vertexNormal;

layout(location = 0) in vec4 vertexCenter;
layout(location = 1) in vec4 vertexColor;

uniform mat4 transformation;
uniform mat4 modelMatrix;

out vec4 color;
out vec3 interpNormal;
out vec3 fragPos;

void main()
{
	vec4 worldSpaceNormal = modelMatrix * vec4(vertexNormal, 0);
	interpNormal = vec3(worldSpaceNormal);
	vec4 worldSpaceVertex = modelMatrix * vec4(vertexPosition, 1.0);
	fragPos = vec3(worldSpaceVertex);

	color = vertexColor;
	float scale = vertexCenter.w;
	mat4 scaleMatrix = mat4(vec4(scale,0,0,0),vec4(0,scale,0,0),vec4(0,0,scale,0),vec4(0,0,0,1));
	gl_Position = transformation * scaleMatrix * vec4(vertexPosition + vertexCenter.xyz, 1.0);

}