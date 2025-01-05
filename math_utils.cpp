#include "math_utils.hpp"

WallSide getWallSide(glm::vec3 point, room_wall wall) 
{
    float d = glm::dot(point - wall.wall_plane.origin, wall.wall_plane.normal);
    
    if (d > 0.0f) 
    { 
        return WallSide::FRONT;
    }
    
    return WallSide::BACK;
}

float sqrMag(const glm::vec2 & v) { return v.x * v.x + v.y * v.y;}
float sqrMag(const glm::vec3 & v) { return v.x * v.x + v.y * v.y + v.z * v.z;};

bool aabb_contains(glm::vec3 & pos, const AABB & bb)
{
    float xDist = glm::abs(pos.x - bb.center.x);
    float yDist = glm::abs(pos.y - bb.center.y);
    float zDist = glm::abs(pos.z - bb.center.z);
    return (xDist <= bb.extents.x) && (yDist <= bb.extents.y) && (zDist <= bb.extents.z);
}

#define EPS 1e-4f

bool distance_to_plane(const ray_t & ray, const plane_t & plane, float* distance)
{
    float denom = glm::dot(plane.normal, ray.direction);
    if (glm::abs(denom) > EPS) 
    { 
        *distance = glm::dot(plane.normal, plane.origin - ray.origin) / denom;
        return true;
    }
    return false;
}
