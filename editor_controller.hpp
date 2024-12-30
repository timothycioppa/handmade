#ifndef _EDITOR_CONTROLLER_HPP
#define _EDITOR_CONTROLLER_HPP

#include "platform_common.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "game_context.hpp"

struct editor_player
{ 
    bool shouldMove;
    glm::vec3 Position;
    glm::vec3 Forward;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::vec3 MoveDir;
    float MovementSpeed;
    camera_data camData;
};

extern editor_player gEditorPlayer;

void EditorPlayer_Init(game_context * context);
void EditorPlayer_UpdatePosition(game_context * context); 
void EditorPlayer_UpdateMovementDirection(game_context * context);

#endif