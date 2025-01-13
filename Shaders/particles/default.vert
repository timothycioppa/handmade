
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec2 ageLifetime; 
layout (location = 4) in mat4 aModel;

uniform mat4 projection;
uniform mat4 view;

out vec2 TexCoords;
out float age;
out float lifetime;
out float normalizedAge;

void main()
{
    gl_Position = projection * view * aModel * vec4(aPos, 1.0); 
    TexCoords = aTexCoords;
    age = ageLifetime.x;
    lifetime = ageLifetime.y;
    normalizedAge = age / lifetime;
}