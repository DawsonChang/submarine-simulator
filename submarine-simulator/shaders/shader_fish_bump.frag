#version 430 core

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform vec3 submarinePos;

in vec3 lightDirTS;
in vec3 submarinePosTS;
in vec3 viewDirTS;
in vec2 interpTexCoord;
in vec4 fragPos;

float fogMaxDist = 250.0;
float fogMinDist = 30.0;
vec4  fogColor = vec4(0.9, 1.0, 1.0, 1.0);
bool visible = true;

void main()
{
	vec2 texCoord_flipped = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y);
	
	// light from submarine
	vec3 L = -normalize(submarinePosTS);
	vec3 V = normalize(viewDirTS);
	vec3 N = 2 * texture2D(normalTexture, texCoord_flipped).rgb - 1;
	N = normalize(N);
	 
	vec3 R = reflect(L, N);
	float diffuse = max(0, dot(N, L));
	float specular_pow = 10;
	float specular = pow(max(0, dot(R, V)), specular_pow);

	vec3 color = texture2D(colorTexture, texCoord_flipped).rgb;

	vec3 lightColor = vec3(1);
	vec3 shadedColor = color * diffuse + lightColor * specular;
	
	// light from sun
	vec3 L2 = -normalize(lightDirTS);
	vec3 R2 = reflect(L2, N);
	float diffuse2 = max(0, dot(N, L2));
	float specular2 = pow(max(0, dot(R2, V)), specular_pow);

	vec3 shadedColor2 = color * diffuse2 + lightColor * specular2;

	vec4 mixing = vec4(mix(color, shadedColor + shadedColor2, 0.5), 1.0);

	//fog
	float dist = distance(submarinePos, fragPos.xyz);
	if(dist > fogMaxDist - 70.0) {
		visible = false;
	} else {
		visible = true;
	}
    float fogFactor = (fogMaxDist - dist) / (fogMaxDist - fogMinDist);
    fogFactor = clamp(fogFactor, 0.0, 0.8);
	if(visible == true) {
	    gl_FragColor = mix(fogColor, mixing, fogFactor);
	} else {
		gl_FragColor = vec4(mix(color, shadedColor + shadedColor2, 0.5), 0.0);
	}
}
