#ifndef _JC_MATH_H
#define _JC_MATH_H

#include "platform_common.hpp"

enum WallSide { FRONT, BACK };

struct plane_t 
{ 
    glm::vec3 normal;
    glm::vec3 origin;
};

struct room_wall
{
    plane_t wall_plane;
    float width;
    float height;
};

struct room_floor 
{ 
    glm::vec3 origin;
    float width;
    float height;
};

struct room_ceiling 
{ 
    glm::vec3 origin;
    float width;
    float height;
};

WallSide getWallSide(glm::vec3 point, room_wall wall);
float sqrMag(const glm::vec2 & v);
float sqrMag(const glm::vec3 & v);

#endif