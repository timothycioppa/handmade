#ifndef _JC_LIGHT_HPP
#define _JC_LIGHT_HPP

#include "Transform.hpp"

struct light 
{    
    int active; // 0 or 1
    glm::vec3 Color;
    float intensity;
    glm::vec3 Position;
    glm::vec3 Forward;
    glm::vec3 Right;
    glm::vec3 Up;
    glm::vec3 WorldUp;
    float Pitch;
    float Yaw;
    float near;
    float far;
    float frustrumWidth;
    float frustrumHeight;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 lightSpace;
};

void update_light_direction(light & l);
void update_projection_matrix(light & l);

#endif