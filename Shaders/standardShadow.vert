#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 unity_MVP;
uniform mat4 unity_M;
uniform mat4 unity_V;
uniform mat4 unity_MV;
uniform mat4 lightSpaceMatrix;

void main()
{
    vs_out.FragPos = (unity_M * vec4(aPos, 1.0)).xyz;
    vs_out.Normal = transpose(inverse(mat3(unity_M))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = unity_MVP * vec4(aPos, 1.0);
}