#include "math_utils.hpp"
#define EPS 1e-4f

float sqrMag(const glm::vec2 & v) 
{ 
    return v.x * v.x + v.y * v.y;
}

float sqrMag(const glm::vec3 & v) 
{ 
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

bool aabb_contains(const glm::vec3 & pos, const AABB & bb)
{
    float xDist = glm::abs(pos.x - bb.center.x);
    float yDist = glm::abs(pos.y - bb.center.y);
    float zDist = glm::abs(pos.z - bb.center.z);
    return (xDist <= bb.extents.x) && (yDist <= bb.extents.y) && (zDist <= bb.extents.z);
}

bool aabb_walls_contains(const glm::vec3 & pos, const AABB & bb)
{
    float xDist = glm::abs(pos.x - bb.center.x);
    float zDist = glm::abs(pos.z - bb.center.z);
    return (xDist <= bb.extents.x) &&  (zDist <= bb.extents.z);
}

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

float distance_to_plane (const glm::vec3 & point, const plane_t & plane) 
{
    return glm::dot(point - plane.origin, plane.normal);
}

#define LX(aabb) aabb.center.x - aabb.extents.x
#define RX(aabb) aabb.center.x + aabb.extents.x
#define LY(aabb) aabb.center.y - aabb.extents.y
#define RY(aabb) aabb.center.y + aabb.extents.y
#define LZ(aabb) aabb.center.z - aabb.extents.z
#define RZ(aabb) aabb.center.z + aabb.extents.z

bool aabb_intersect(const AABB & first, const AABB & second)
{
    if (LX(first) > RX(second)) { return false; }
    if (LX(second) > RX(first)) { return false; }
    if (LY(first) > RY(second)) { return false; }
    if (LY(second) > RY(first)) { return false; }
    if (LZ(first) > RZ(second)) { return false; }
    if (LZ(second) > RZ(first)) { return false; }
    return true;
}
