#ifndef _GAMEPLAY_CONTEXT_HPP
#define _GAMEPLAY_CONTEXT_HPP

#include "../../platform_common.hpp"
#include "../../particle_system.hpp"
#include "../../Material.hpp"

#define MAX_PROJECTILES 4
#define MAX_ENTITIES 10

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

struct game_entity 
{
    float MoveSpeed;
    Transform transform;
    Material material;
    AABB boundingBox;
};

struct gameplay_context 
{ 
	weapon primaryWeapon;
    glm::vec3 weaponAimDir;
    Transform weaponTransform;
    Material weaponMaterial;
    
    particle_system explosion;
    unsigned int numEntities;
    game_entity Entities[MAX_ENTITIES];
};

#endif