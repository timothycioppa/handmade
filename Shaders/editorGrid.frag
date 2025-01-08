#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

float distToBoundary(vec2 pos)
{
    float distX = min(pos.x, 1.0f - pos.x);
    float distY = min(pos.y, 1.0f - pos.y);
    return min(distX, distY);
}

uniform vec2 cameraPosition;
uniform mat4 clipToWorld;

void main()
{   
    vec3 backgroundColor = vec3(0,0,0.15f); 
    vec3 gridColor = vec3(0.4f, 0.4f, 0.4f);

    vec4 clipPos = vec4(2.0f * (TexCoords - 0.5f), -1.0f, 1.0f);
    vec2 worldPos = (clipToWorld * clipPos).xz; 
    vec2 gridCoords =  worldPos - floor( worldPos);
    vec3 finalColor = backgroundColor;
    float thresh = 0.01f;

    if (distToBoundary(gridCoords) < thresh)
    {
        finalColor = gridColor;
    }
        
    FragColor = vec4(finalColor,  1.0);
}