#ifndef _JC_MATH_H
#define _JC_MATH_H

#include "platform_common.hpp"

enum WallSide { FRONT, BACK };

struct plane_t 
{ 
    glm::vec3 origin;
    glm::vec3 normal;
};

struct ray_t
{
    glm::vec3 origin;
    glm::vec3 direction;
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

struct AABB
{
    glm::vec3 center;
    glm::vec3 extents;
};



WallSide getWallSide(glm::vec3 point, room_wall wall);
float sqrMag(const glm::vec2 & v);
float sqrMag(const glm::vec3 & v);
bool aabb_contains(glm::vec3 & pos, const AABB & bb);
bool distance_to_plane(const ray_t & ray, const plane_t & plane, float* distance);

#endif