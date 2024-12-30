#include "LevelEditor.hpp"

void LevelEditor_Init(game_context & context) 
{
	EditorPlayer_Init(&context);
}

void LevelEditor_Update(game_context & context) 
{
    glm::vec3 lineColor = glm::vec3(1,1,1);

    debug_line(glm::vec3(0,0,0), glm::vec3(1,0,0), lineColor, gEditorPlayer.camData);
    debug_line(glm::vec3(1,0,0), glm::vec3(1,0,1), lineColor, gEditorPlayer.camData);
    debug_line(glm::vec3(1,0,1), glm::vec3(0,0,1), lineColor, gEditorPlayer.camData);
    debug_line(glm::vec3(0,0,1), glm::vec3(0,0,0), lineColor, gEditorPlayer.camData);

    EditorPlayer_UpdateMovementDirection(&context);
    EditorPlayer_UpdatePosition(&context);    
}

void LevelEditor_Render(game_context & context) 
{
    G_RenderLevelEditor();
}

void LevelEditor_Editor(game_context & context) 
{}

void LevelEditor_Destroy(game_context & context) 
{}

game_state gStateLevelEditor = 
{
    LevelEditor_Init,
    LevelEditor_Update,
    LevelEditor_Render,
    LevelEditor_Editor,
    LevelEditor_Destroy,
    false, 
    GameState::NONE
};