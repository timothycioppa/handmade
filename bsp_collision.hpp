#ifndef BSP_COLLISION_HPP
#define BSP_COLLISION_HPP

#include "bsp.hpp"


enum BoxSide 
{ 
    BS_TOP = 0, BS_BOTTOM = 1, BS_RIGHT = 2, BS_LEFT = 3, BS_FRONT = 4, BS_BACK = 5
};

struct boxIntersection 
{ 
    glm::vec3 point;
    BoxSide side;
};

struct planeIntersection 
{ 
    glm::vec3 point;
};


bool try_get_intersection(const ray_t & ray, const AABB & bb, boxIntersection & intersection);
bool try_get_intersection(const ray_t & ray, const plane_t & plane, planeIntersection & intersection) ;
bool bsp_raycast(glm::vec3 origin, glm::vec3 direction, raycast_hit & hit, bsp_tree & tree) ;
bool test_pos_bsp(const glm::vec3 & testPos, bsp_tree & tree) ;

#endif