#version 330 core
layout (location = 0) in vec3 vertexPosition;

out vec3 TexCoords;

uniform mat4 transformation;
uniform mat4 modelMatrix;

void main()
{
    TexCoords = vertexPosition;
    gl_Position = transformation * modelMatrix * vec4(vertexPosition, 1.0);
}  