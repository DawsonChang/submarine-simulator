#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;
uniform float time;

uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform vec3 submarinePos;


out vec3 lightDirTS;
out vec3 submarinePosTS;
out vec3 viewDirTS;
out vec2 interpTexCoord;
out vec4 fragPos;


void main()
{
	vec3 vertPos = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;
	vec3 vertex = vertexPosition;

	vertex.y += cos(time) * 0.55f;
	float pivot_angle = cos(time) * 0.1 * -0.7f;
	mat2 rotation_matrix = mat2(vec2(cos(pivot_angle), -sin(pivot_angle)), vec2(sin(pivot_angle), cos(pivot_angle)));
	vertex.xy = rotation_matrix * vertex.xy;
	float body = (vertex.x + 1.0) / 2.0;
	vertex.y += cos(time + body) * 0.55f;
	
	
	gl_Position = transformation * vec4(vertex, 1.0);
	fragPos = transformation * vec4(vertex, 1.0);

	// bump mapping
	// calculate the TBN matrix for bump mapping
	vec3 N = (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
	vec3 T = (modelMatrix * vec4(aTangent, 0.0)).xyz;
	vec3 B = (modelMatrix * vec4(aBitangent, 0.0)).xyz;

	mat3 TBN = transpose(mat3(T, B, N));

	vec3 viewDir = normalize(cameraPos - vertPos);

	lightDirTS = TBN * (lightPos - vertPos);
	submarinePosTS = TBN * (submarinePos - vertPos);
	viewDirTS = TBN * viewDir;

	interpTexCoord = vertexTexCoord;
}
