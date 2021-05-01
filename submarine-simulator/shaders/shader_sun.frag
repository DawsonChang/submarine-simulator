#version 430 core

uniform vec3 objectColor;
uniform vec3 cameraPos;
uniform sampler2D colorTexture;

in vec3 interpNormal;
in vec3 fragPos;
in vec2 texCoord;

void main()
{
	vec4 textureColor = texture2D(colorTexture, texCoord);
	gl_FragColor = textureColor;

	// I.7.* Change sun shader to more realistic.
	//vec3 V = normalize(fragPos - cameraPos);
	//float cos = dot(V, interpNormal) / (length(V) * length(interpNormal)) / 1.5;
	//gl_FragColor = vec4(objectColor.x, objectColor.y - cos, objectColor.z, 1.0);
}
