#version 430 core

// I.5. change directional lighting to point lightning


uniform sampler2D colorTexture;
uniform sampler2D normalTexture;

in vec3 fragPosTS;
in vec3 lightPosTS;
in vec3 submarinePosTS;
in vec3 cameraPosTS;
in vec2 texCoord;

void main()
{
	
	
	// II.3.c textureColor
	vec2 texCoord_flipped = vec2(texCoord.x, 1.0 - texCoord.y);
	vec3 textureColor = texture2D(colorTexture, texCoord_flipped).rgb;
	vec3 textureNormal = texture2D(normalTexture, texCoord_flipped).rgb;
	textureNormal = normalize((textureNormal - 0.5) * 2.0);

	// I.5. change directional lighting to point lightning
	vec3 lightDir = normalize(fragPosTS - lightPosTS);

	// I.1. Calculate diffuse Phong lightning intensity
	float intensity = max(dot(textureNormal, -lightDir), 0);
	//float fMult = clamp(intensity, 0.0, 1.0);

	// II.3.c change the objectColor to textureColor
	vec3 diffuseColorSun = textureColor * intensity * 0.7;

	
	// I.3. Add specular Phong lighting
	vec3 V = normalize(fragPosTS - cameraPosTS);
	vec3 R = reflect(textureNormal, -lightDir);
	float intensityR = max(dot(V, R), 0);
	vec3 specularColorSun = vec3(0.5) * intensityR * 0.7;
	
	// light from submarine
	// I.5. change directional lighting to point lightning
	lightDir = normalize(fragPosTS - submarinePosTS);

	// I.1. Calculate diffuse Phong lightning intensity
	intensity = max(dot(textureNormal, -lightDir), 0);

	// II.3.c change the objectColor to textureColor
	vec3 diffuseColorSubmarine = vec3(textureColor) * intensity * 0.7;

	// I.3. Add specular Phong lighting
	V = normalize(fragPosTS - cameraPosTS);
	R = reflect(textureNormal, -lightDir);
	intensityR = max(dot(V, R), 0);
	vec3 specularColorSubmarine = vec3(0.5) * intensityR * 0.7;
	
	gl_FragColor = vec4(diffuseColorSun + specularColorSun + diffuseColorSubmarine + specularColorSubmarine, 1.0);
	
}
