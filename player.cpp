#include "player.hpp"
#include "game_data.hpp"
#include "math_utils.hpp"
#include <stdio.h>
#include "include/glm/gtc/quaternion.hpp"

player_data main_player;

void update_view_matrix(player_data* player)
{ 
    float yawRad = glm::radians(player->Horizontal);
    float pitchRad = glm::radians(player->Vertical);
    glm::vec3 front;
    
    front.x = cos(yawRad) * cos(pitchRad);
    front.y = sin(pitchRad);
    front.z = sin(yawRad) * cos(pitchRad);

    player->Forward = glm::normalize(front);
    player->Right = glm::normalize(glm::cross(player->WorldUp, player->Forward));  
    player->Up    = glm::normalize(glm::cross( player->Forward, player->Right));
    player->camData.view =  glm::lookAt(player->Position, player->Position + player->Forward, {0,1,0});        
}

void Player_Init(game_context * context) 
{ 
    main_player.Horizontal = 0.0f;
    main_player.Vertical = 0.0f;
    main_player.MovementSpeed = 15.0f;
    main_player.LookSpeed = 200.0f;
    main_player.Position = {0,2,0};
    
    main_player.boundingBox = 
    {
        glm::vec3(0, 1.5f, 0),
        glm::vec3(1, 1.5f, 1)
    };

    main_player.WorldUp = {0,1,0};
    main_player.Jumping = false;
    main_player.camData.projection = glm::perspective(glm::radians(45.0f), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 200.0f);

  
    update_view_matrix(&main_player);
}

float clampf(float value, float min, float max) 
{ 
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

#define CLAMPF(v,m,M) if (v < m) {v = m;} if (v > M) {v = M;}

void Player_UpdateView(game_context * context) 
{ 
    float xoffset = context->mouseDelta.x;
    float yoffset = context->mouseDelta.y;
    
    xoffset *= main_player.LookSpeed;
    yoffset *= main_player.LookSpeed;

    main_player.Horizontal   -= xoffset;
    main_player.Vertical += yoffset;
    CLAMPF(main_player.Vertical, -89.0f, 89.0f)
    update_view_matrix(&main_player);
}

void Player_UpdateMovementDirection(game_context * context) 
{ 
    glm::vec3 & direction = main_player.MoveDir;
    direction = {0,0,0};

    if (key_held(KeyCodes::KEY_E)) 
    { 
        direction += main_player.Forward; 
    }

    if (key_held(KeyCodes::KEY_D)) 
    {
        direction -= main_player.Forward; 
    }
    
    if (key_held(KeyCodes::KEY_F)) 
    { 
        direction += main_player.Right; 
    }
    
    if (key_held(KeyCodes::KEY_S)) 
    { 
       direction -= main_player.Right;
    }

    direction.y = 0.0f;

    if (glm::dot(direction, direction) < .1f ) 
    { 
        direction = {0,0,0};
    } else 
    {
        direction = glm::normalize(direction);
    }
}

void Player_UpdatePosition( game_context * context) 
{ 
    if (sqrMag(main_player.MoveDir) > 0.1f) 
    { 
        float velocity = main_player.MovementSpeed * (context->deltaTime);
        main_player.Position += velocity * main_player.MoveDir;
        main_player.camData.view = glm::lookAt(main_player.Position, main_player.Position + main_player.Forward, main_player.Up);
        main_player.boundingBox.center = main_player.Position;
        main_player.boundingBox.center.y -= 0.5f;
    } 
}