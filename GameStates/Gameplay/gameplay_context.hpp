#ifndef _GAMEPLAY_CONTEXT_HPP
#define _GAMEPLAY_CONTEXT_HPP

#include "../../platform_common.hpp"
#include "../../particle_system.hpp"

#define MAX_PROJECTILES 15

struct projectile 
{ 
    bool active;
    float age;
    float lifetime;
    glm::vec3 position;
    glm::vec3 direction;
};

struct weapon 
{ 
    bool ableTofire;
    float fireDelay;
    float fireTimer;
    float projectileSpeed;
    projectile projectiles[MAX_PROJECTILES];
};


struct gameplay_context 
{ 
	weapon primaryWeapon;
    particle_system explosion;
};

#endif