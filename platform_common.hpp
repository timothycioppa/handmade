#ifndef _PLATFORM_COMMON_HPP
#define _PLATFORM_COMMON_HPP

#include "include/glad/glad.h"
#include "include/glfw/glfw3.h"
#include "include/glm/glm.hpp"
#include "include/glm/gtc/type_ptr.hpp"

struct camera_data 
{
    glm::mat4 projection;
    glm::mat4 view;
};

#endif