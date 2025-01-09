#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D colorBuffer;

void main()
{   
   float eps = 0.01f;          

   vec3 colorUp = texture(colorBuffer, TexCoords + vec2(0,eps)).rgb;    
   vec3 colorDown = texture(colorBuffer, TexCoords - vec2(0, eps)).rgb;    
   vec3 colorLeft = texture(colorBuffer,  TexCoords - vec2(eps, 0)).rgb;    
   vec3 colorRight = texture(colorBuffer, TexCoords + vec2(eps, 0)).rgb;    
   
   vec3 color = 0.25 * (colorUp + colorDown + colorLeft + colorRight);
   FragColor = vec4(color + 0.2f, 1.0);
}