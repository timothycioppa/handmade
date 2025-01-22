#ifndef GL_RENDER_CONTEXT_HPP
#define GL_RENDER_CONTEXT_HPP

#include "../platform_common.hpp"

struct RenderContext 
{ 
    glm::mat4 v;   // VIEW
    glm::mat4 p;   // PROJECTION
    glm::mat4 lightSpace;   // LIGHT SPACE
    glm::vec3 cameraPosition;  // CAMERA POS
        glm::vec3 cameraForward;  // CAMERA POS

    float deltaTime, totalTime, sinTime, cosTime;
    unsigned int shadowMapID;
    glm::vec3 lightColor;
	float lightPower;
	glm::vec3 lightPosition;
};

#endif