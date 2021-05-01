#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;
uniform vec3 lightPos;
uniform vec3 submarinePos;
uniform vec3 cameraPos;

out vec3 fragPosTS;
out vec3 lightPosTS;
out vec3 submarinePosTS;
out vec3 cameraPosTS;
out vec2 texCoord;

void main()
{
	texCoord = vec2(vertexTexCoord.x, vertexTexCoord.y);

	// calculate the TBN matrix for bump mapping
	vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(vertexNormal, 0.0)));

	mat3 TBN = transpose(mat3(T, B, N));

	lightPosTS = TBN * lightPos;
	submarinePosTS = TBN * submarinePos;
    cameraPosTS  = TBN * cameraPosTS;
    fragPosTS  = TBN * vec3(modelMatrix * vec4(vertexPosition, 1.0));
	
	gl_Position = transformation * vec4(vertexPosition, 1.0);
}
