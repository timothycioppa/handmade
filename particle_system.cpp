#include "particle_system.hpp"
#include "Graphics/GLMesh.hpp"
#include "ShaderStore.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "player.hpp"
#include "TextureStore.hpp"
#include "r_main.hpp"

unsigned int matrixBufferID;
unsigned int ageBufferID;
static_mesh _quadMesh;
texture_info texInfo;
float gravity = -1.0f;

void simulate_system(particle_system & system,particle_simulation_context & context)
{    
    int liveParticles = NUM_PARTICLES;
  
    system.age += context.deltaTime;

    for(int i = 0; i < NUM_PARTICLES; i++) 
    {
        particle_state & particle = system.states[i];
        

        if (particle.alive) 
        {
            particle.velocity = particle.initial_velocity + gravity * particle.age * glm::vec3(0,1,0);
            particle.position = particle.initial_position + particle.age * particle.velocity;
            particle.age += context.deltaTime; 

            system.ageLifetime[i] = { particle.age, particle.lifetime };

            glm::vec3 direction = glm::normalize(context.cameraPosition - particle.position); 
            float size = 12.0f * (1.0f - particle.age / particle.lifetime);
            system.matrices[i] = glm::inverse(glm::lookAt(particle.position, particle.position + direction, glm::vec3(0,1,0) )) * glm::scale(glm::mat4(1.0f), glm::vec3(size, size, size));

            if (particle.age > particle.lifetime) 
            {
                particle.alive = false;
            }
        }
         else 
        { 
            liveParticles--;
        }
    }

    if (liveParticles == 0) 
    {
        system.alive = false;
    }
}



void init_system(particle_system & system, system_spawn_info &info) 
{    
    system.alive = true;
    system.age = 0.0f;
    
    for (int i = 0; i < NUM_PARTICLES; i++) 
    {
        particle_state & particle = system.states[i];

        particle.alive = true;
        particle.lifetime = random(0.25f, 0.5f);
        particle.age = 0.0f;
        particle.position = info.initialPosition,
        particle.velocity = glm::vec3(0.0f);
        particle.initial_velocity = 5.0f * random_unit_vector();
        particle.initial_position = info.initialPosition,
        system.matrices[i] = glm::mat4(1.0f);
        system.ageLifetime[i] = {0.0f, 0.0f};
    }
}

void init_particles(const char* meshFile) 
{     
    load_texture("Textures/flamesheet.jpg", texInfo);
    load_mesh(meshFile, _quadMesh);

    // 1. generate buffer to hold instance matrix data
    glGenBuffers(1, &matrixBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, matrixBufferID);
    glBufferData(GL_ARRAY_BUFFER, NUM_PARTICLES * sizeof(glm::mat4), NULL, GL_STREAM_DRAW );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  
    glGenBuffers(1, &ageBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, ageBufferID);
    glBufferData(GL_ARRAY_BUFFER, NUM_PARTICLES * sizeof(glm::vec2), NULL, GL_STREAM_DRAW );
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindVertexArray(_quadMesh.VAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, ageBufferID);
        {
            // age and lifetime array
            glEnableVertexAttribArray(3); 
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*) 0 );
          
            // one age/lifetime value per instance
            glVertexAttribDivisor(3, 1);
        }

        glBindBuffer(GL_ARRAY_BUFFER, matrixBufferID);
        {
            // model matrix array
            glEnableVertexAttribArray(4); 
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(0 * sizeof(glm::vec4)));
            
            glEnableVertexAttribArray(5); 
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)));
            
            glEnableVertexAttribArray(6); 
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
            
            glEnableVertexAttribArray(7); 
            glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));
            
            // one model matrix value per instance
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);
            glVertexAttribDivisor(7, 1);
        }

        
    }    
    glBindVertexArray(0);
}

#define uniform_data(s, T) *((T*)s.uniformIDS)

void render_system(particle_system & system, camera_data & camData) 
{   

    glBindBuffer(GL_ARRAY_BUFFER, matrixBufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, NUM_PARTICLES * sizeof(glm::mat4), &(system.matrices[0]));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, ageBufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, NUM_PARTICLES * sizeof(glm::vec2), &(system.ageLifetime[0]));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    shader_data sData = bind_shader(ShaderCode::DEFAULT_PARTICLE);
    default_particle_ids & ids = uniform_data(sData, default_particle_ids);

    set_mat4(ids.projection, camData.projection);
    set_mat4(ids.view, camData.view);
    set_texture(ids.mainTex, texInfo.textureID, 0);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    R_DrawMeshInstanced(_quadMesh, NUM_PARTICLES);    
    unbind_shader();
}