#version 330 core

in vec3 TexCoords;

uniform samplerCube skyboxTexture;
uniform vec3 submarinePos;

vec4  fogColor = vec4(0.9, 1.0, 1.0, 1.0);

void main()
{    
    vec3 coords = TexCoords;
    gl_FragColor = mix(texture(skyboxTexture, TexCoords), fogColor, 0.6);
}