#ifndef _PLAYER_HPP
#define _PLAYER_HPP
#include "platform_common.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "game_context.hpp"

struct player_data 
{ 
    glm::vec3 Position;
    glm::vec3 Forward;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::vec3 MoveDir;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float LookSpeed;
};

extern player_data main_player;

void Player_Init(game_context * context);
void Player_UpdateView(game_context * context);
void Player_UpdatePosition(game_context * context); 
void Player_UpdateMovementDirection(game_context * context);

#endif