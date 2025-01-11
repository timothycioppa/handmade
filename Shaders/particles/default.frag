#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

void main()
{             
    vec3 color = vec3(1,1,1);
    FragColor = vec4(color, 1.0);
}