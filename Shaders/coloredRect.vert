#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform vec3 lowerLeft;
uniform vec3 dimensions;

void main()
{
    gl_Position = vec4(lowerLeft + dimensions * vec3(aTexCoords, 0.0f), 1.0); 
}  