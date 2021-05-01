#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 transformation;
uniform mat4 modelMatrix;

uniform float time;

out vec3 interpNormal;
out vec3 fragPos;

void main()
{
	// I.2. spaceship illumination
	vec4 worldSpaceNormal = modelMatrix * vec4(vertexNormal, 0);
	interpNormal = vec3(worldSpaceNormal);

	// I.3. Add specular Phong lighting
	vec4 worldSpaceVertex = modelMatrix * vec4(vertexPosition, 1.0);
	fragPos = vec3(worldSpaceVertex);

	vec3 vertex = vertexPosition;
	vertex.x += cos(time) * 0.55f;
	float pivot_angle = cos(time) * 0.1 * 0.7f;
	mat2 rotation_matrix = mat2(vec2(cos(pivot_angle), -sin(pivot_angle)), vec2(sin(pivot_angle), cos(pivot_angle)));
	vertex.xz = rotation_matrix * vertex.xz;
	float body = (vertex.z + 1.0) / 2.0;
	vertex.x += cos(time + body) * 0.3f;
	float twist_angle = cos(time + body) * 0.3 * 0.5f;
	mat2 twist_matrix = mat2(vec2(cos(twist_angle), -sin(twist_angle)), vec2(sin(twist_angle), cos(twist_angle)));
	vertex.xy = twist_matrix * vertex.xy;

	gl_Position = transformation * vec4(vertex, 1.0);
}
