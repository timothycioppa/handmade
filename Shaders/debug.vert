#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 unity_MVP;
uniform mat4 unity_M;
uniform mat4 unity_V;
uniform mat4 unity_MV;
uniform float totalTime;

out vec2 uv;


void main()
{
   // clip position of fragment 
   gl_Position = unity_MVP * vec4(aPos, 1);
   uv = aUV;
}