#version 430 core

uniform samplerCube skyboxTexture;
uniform vec3 submarinePos;

in vec4 color;
in vec3 interpNormal;
in vec3 fragPos;

vec4 fog_colour = vec4(0.9, 1.0, 1.0, 1.0);

void main()
{
	vec3 lightDir = normalize(fragPos - submarinePos);
	vec3 light = normalize(lightDir);
	vec3 nor = normalize(interpNormal);
	float ratio = 1.00 / 1.52;
	vec3 R = refract(-light, nor, ratio);
	vec4 result = vec4(texture(skyboxTexture, -R).rgb, 0.5);
	result.a = 0.4;

    gl_FragColor = mix(result, fog_colour, 0.45);
}
