#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 unity_MVP;
uniform mat4 unity_M;
uniform mat4 unity_V;
uniform mat4 unity_MV;
uniform vec3 lightPositionWS;

out vec4 positionWS;
out vec3 normalCameraSpace;
out vec3 eyeDirectionCameraSpace;
out vec2 uv;
out vec3 lightDirectionCameraSpace;

void main()
{
   // clip position of fragment 
   gl_Position = unity_MVP * vec4(aPos, 1);

   // world position of vertex
   positionWS = unity_M * vec4(aPos, 1);

   // world normal
   normalCameraSpace = (unity_MV * vec4(aNormal, 0)).xyz;
   uv = aUV;
   
   vec3 cameraSpaceVertex = (unity_MV * vec4(aPos, 1)).xyz;
   eyeDirectionCameraSpace = vec3(0,0,0) - cameraSpaceVertex;
   
   vec3 lightPosCameraSpace = (unity_V * vec4(lightPositionWS, 1)).xyz;
   lightDirectionCameraSpace =  eyeDirectionCameraSpace;

}