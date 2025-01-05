#include "player.hpp"
#include "game_data.hpp"
#include "math_utils.hpp"
#include <stdio.h>
player_data main_player;

void update_view_matrix(player_data* player)
{ 
    glm::vec3 front;

    float yawRad = glm::radians(player->Yaw);
    float pitchRad = glm::radians(player->Pitch);

    front.x = cos(yawRad) * cos(pitchRad);
    front.y = sin(pitchRad);
    front.z = sin(yawRad) * cos(pitchRad);

    player->Forward = glm::normalize(front);
    player->Right = glm::normalize(glm::cross(player->Forward, player->WorldUp));  
    player->Up    = glm::normalize(glm::cross(player->Right, player->Forward));

    player->camData.view = glm::lookAt(player->Position, player->Position + player->Forward, {0,1,0});
        
}

void Player_Init(game_context * context) 
{ 
    main_player.Pitch = 0.0f;
    main_player.Yaw = 0.0f;
    main_player.MovementSpeed = 15.0f;
    main_player.LookSpeed = 200.0f;
    main_player.Position = {0,2,0};
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

    main_player.Yaw   += xoffset;
    main_player.Pitch += yoffset;
    CLAMPF(main_player.Pitch, -89.0f, 89.0f)

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
            main_player.camData.view = glm::lookAt(main_player.Position,main_player.Position + main_player.Forward, {0,1,0});

    } 
}