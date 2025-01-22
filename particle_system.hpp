#ifndef _PARTICLE_SYSTEM_HPP
#define _PARTICLE_SYSTEM_HPP
#include "platform_common.hpp"

#define NUM_PARTICLES 45

struct particle_state 
{ 
    bool alive = true;
    float lifetime;
    float age;
    glm::vec3 initial_velocity;
    glm::vec3 velocity;
    glm::vec3 initial_position;
    glm::vec3 position;
};

struct particle_system 
{ 
    bool alive;
    float age;
    particle_state states[NUM_PARTICLES];
    glm::mat4 matrices[NUM_PARTICLES];
    glm::vec2 ageLifetime[NUM_PARTICLES];
};

struct system_spawn_info 
{ 
    glm::vec3 initialPosition;
    glm::vec2 ageRange;
    glm::vec2 speedRange;
};

struct particle_simulation_context
{
    float deltaTime;
    glm::vec3 cameraPosition;
};

void init_particles(const char* meshFile);
void init_system(particle_system & system, system_spawn_info & info);
void render_system(particle_system & system, camera_data & camData) ;
void simulate_system(particle_system & system, particle_simulation_context & context);

#endif