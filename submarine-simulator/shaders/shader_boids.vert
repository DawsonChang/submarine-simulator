#version 430 core

layout(location = 2) in vec3 vertexPosition;
layout(location = 3) in vec2 vertexTexCoord;
layout(location = 4) in vec3 vertexNormal;

layout(location = 0) in vec4 vertexCenter;
layout(location = 1) in vec4 vertexRotate;

uniform mat4 transformation;

out vec3 interpNormal;
out vec3 fragPos;
out vec2 texCoord;

void main()
{
	mat4 modelMatrix;
	modelMatrix[0][0] = 1.0;
	modelMatrix[3][0] = vertexCenter.x;

	modelMatrix[1][1] = 1.0;
	modelMatrix[3][1] = vertexCenter.y;

	modelMatrix[2][2] = 1.0;
	modelMatrix[3][2] = vertexCenter.z;

	modelMatrix[3][3] = 1.0;

	float angleBoid;
	if (vertexRotate.y > 0) angleBoid = vertexRotate.w;
	else angleBoid = -vertexRotate.w;
	mat4 rotateMatrix;
	rotateMatrix[0][0] = cos(angleBoid);
	rotateMatrix[0][2] = -sin(angleBoid);
	rotateMatrix[1][1] = 1;
	rotateMatrix[2][0] = sin(angleBoid);
	rotateMatrix[2][2] = cos(angleBoid);
	rotateMatrix[3][3] = 1;

	float scale = vertexCenter.w;
	mat4 scaleMatrix = mat4(vec4(scale,0,0,0),vec4(0,scale,0,0),vec4(0,0,scale,0),vec4(0,0,0,1));

	modelMatrix = modelMatrix * rotateMatrix;
	
	vec4 worldSpaceNormal = modelMatrix * vec4(vertexNormal, 0);
	interpNormal = vec3(worldSpaceNormal);
	vec4 worldSpaceVertex = modelMatrix * vec4(vertexPosition, 1.0);
	fragPos = vec3(worldSpaceVertex);
	

	gl_Position = transformation * modelMatrix * scaleMatrix * vec4(vertexPosition, 1.0);

	texCoord = vertexTexCoord;
}