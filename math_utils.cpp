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
