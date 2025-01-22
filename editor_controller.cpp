#include "editor_controller.hpp"
#include "game_data.hpp"

editor_player gEditorPlayer;

void EditorPlayer_Init(game_context * context)
{
    gEditorPlayer.shouldMove = false;
    gEditorPlayer.Position = {0,1,0};
    gEditorPlayer.Right = {1, 0, 0};
    gEditorPlayer.Forward = {0, 1, 0};
    gEditorPlayer.Up = {0, 0, 1};
    gEditorPlayer.WorldUp = {0, 1, 0};
    gEditorPlayer.MovementSpeed = 10.0f;
    gEditorPlayer.MoveDir = {0,0,0};

    gEditorPlayer.camData.view = glm::lookAt(gEditorPlayer.Position, gEditorPlayer.Position + gEditorPlayer.Forward, gEditorPlayer.Up);

    float aspecRatio = float(WINDOW_WIDTH_RES_X) / float(WINDOW_HEIGHT_RES_Y);
    float height = 100.0f;
    float width = aspecRatio * height;

    gEditorPlayer.camData.projection = glm::ortho(-width, width, -height, height, 0.01f, 100.0f);
}

void EditorPlayer_UpdatePosition(game_context * context)
{
    glm::vec3 & direction = gEditorPlayer.MoveDir;
    direction = {0,0,0};

    if (key_held(KeyCodes::KEY_E)) 
    { 
        direction += gEditorPlayer.Up; 
    }

    if (key_held(KeyCodes::KEY_D)) 
    {
        direction -= gEditorPlayer.Up; 
    }
    
    if (key_held(KeyCodes::KEY_F)) 
    { 
        direction -= gEditorPlayer.Right; 
    }
    
    if (key_held(KeyCodes::KEY_S)) 
    { 
       direction += gEditorPlayer.Right;
    }

    if (glm::dot(direction, direction) < 0.1f ) 
    {   
        gEditorPlayer.shouldMove = false; 
        direction = {0,0,0};
    } else 
    {
        gEditorPlayer.shouldMove = true;
        direction = glm::normalize(direction);
    }
}

void EditorPlayer_UpdateMovementDirection(game_context * context)
{
    if (gEditorPlayer.shouldMove) 
    { 
        gEditorPlayer.Position += context->deltaTime * gEditorPlayer.MovementSpeed * gEditorPlayer.MoveDir;
        gEditorPlayer.camData.view = glm::lookAt(gEditorPlayer.Position, gEditorPlayer.Position + gEditorPlayer.Forward, gEditorPlayer.Up);
    }
}