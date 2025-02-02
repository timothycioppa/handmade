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



float sqrMag(const glm::vec2 & v);
float sqrMag(const glm::vec3 & v);

// returns true if the box contains the point, false otherwise
bool aabb_contains(const glm::vec3 & pos, const AABB & bb);

// returns true if the box (ignoring the top/bottom) contains the point, false otherwise
bool aabb_walls_contains(const glm::vec3 & pos, const AABB & bb);

bool aabb_intersect(const AABB & first, const AABB & second);

bool distance_to_plane(const ray_t & ray, const plane_t & plane, float* distance);
float distance_to_plane (const glm::vec3 & point, const plane_t & plane) ;

float random(float min, float max);
glm::vec3 random_unit_vector() ;
glm::vec3 random(glm::vec3 min, glm::vec3 max) ;

#endif