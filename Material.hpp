#ifndef _MATERIAL_HPP
#define _MATERIAL_HPP

#include "Graphics/GLTexture.hpp"

struct Material 
{ 
    texture_info* mainTexture;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

#endif