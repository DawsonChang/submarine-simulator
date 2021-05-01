#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 transformation;
uniform mat4 modelMatrix;

out vec3 interpNormal;
out vec3 fragPos;
out vec2 texCoord;

void main()
{
	// I.2. spaceship illumination
	vec4 worldSpaceNormal = modelMatrix * vec4(vertexNormal, 0);
	interpNormal = vec3(worldSpaceNormal);

	// I.3. Add specular Phong lighting
	vec4 worldSpaceVertex = modelMatrix * vec4(vertexPosition, 1.0);
	fragPos = vec3(worldSpaceVertex);

	gl_Position = transformation * vec4(vertexPosition, 1.0);

	texCoord = vertexTexCoord.st;
}
