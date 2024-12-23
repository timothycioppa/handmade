#version 330 core

in vec2 uv;
out vec4 FragColor;
uniform sampler2D samplerMain;

void main()
{
    FragColor = texture( samplerMain, uv );
}