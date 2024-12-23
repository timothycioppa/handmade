#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D mainTex;

void main()
{ 
    FragColor = texture(mainTex, TexCoords);
}