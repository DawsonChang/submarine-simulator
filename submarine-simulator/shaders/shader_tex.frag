#version 430 core

//uniform vec3 objectColor;

uniform vec3 submarinePos;

// I.5. change directional lighting to point lightning
uniform vec3 lightPos;
uniform vec3 cameraPos;

uniform sampler2D colorTexture;

in vec3 interpNormal;
in vec3 fragPos;
in vec2 texCoord;

float fogMaxDist = 250.0;
float fogMinDist = 30.0;
vec4  fogColor = vec4(0.9, 1.0, 1.0, 1.0);
bool visible = true;

void main()
{
	// light from sun
	// II.4 make texture upside down

	// II.3.c textureColor
	vec2 texCoord_flipped = vec2(texCoord.x, 1.0 - texCoord.y);
	vec4 textureColor = texture2D(colorTexture, texCoord_flipped);

	// I.5. change directional lighting to point lightning
	vec3 lightDir = normalize(fragPos - lightPos);

	// I.1. Calculate diffuse Phong lightning intensity
	vec3 light = normalize(lightDir);
	vec3 nor = normalize(interpNormal);
	float intensity = max(dot(nor, -light), 0);

	// II.3.c change the objectColor to textureColor
	vec3 diffuseColor = vec3(textureColor) * intensity * 0.3;

	// I.3. Add specular Phong lighting
	vec3 V = normalize(fragPos - cameraPos);
	vec3 R = reflect(-light, nor);
	float intensityR = max(dot(V, R), 0);
	vec3 specularColor = vec3(0.5) * intensityR;

	vec3 result = diffuseColor + specularColor;

	// light from submarine
	// I.5. change directional lighting to point lightning
	lightDir = normalize(fragPos - submarinePos);

	// I.1. Calculate diffuse Phong lightning intensity
	light = normalize(lightDir);
	nor = normalize(interpNormal);
	intensity = max(dot(nor, -light), 0);

	// II.3.c change the objectColor to textureColor
	diffuseColor = vec3(textureColor) * intensity * 0.5;

	// I.3. Add specular Phong lighting
	V = normalize(fragPos - cameraPos);
	R = reflect(-light, nor);
	intensityR = max(dot(V, R), 0);
	specularColor = vec3(0.5) * intensityR;

	result = result + diffuseColor + specularColor;

	vec4 res = vec4(result, 1.0);

	//fog
	float dist = distance(submarinePos,fragPos.xyz);
	if(dist > fogMaxDist - 70.0) {
		visible = false;
	} else {
		visible = true;
	}
    float fogFactor = (fogMaxDist - dist) / (fogMaxDist - fogMinDist);
    fogFactor = clamp(fogFactor, 0.0, 0.8);
	if(visible == true) {
		gl_FragColor = mix(fogColor, res, fogFactor);
	} else {
		gl_FragColor = vec4(result, 0.0);
	}
}
