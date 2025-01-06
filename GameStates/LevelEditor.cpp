#include "LevelEditor.hpp"

GAMESTATE_INIT(LevelEditor)
{
	EditorPlayer_Init(&context);
}

GAMESTATE_UPDATE(LevelEditor)
{
    glm::vec3 lineColor = glm::vec3(1,1,1);

    debug_line(glm::vec3(0,0,0), glm::vec3(1,0,0), lineColor, gEditorPlayer.camData);
    debug_line(glm::vec3(1,0,0), glm::vec3(1,0,1), lineColor, gEditorPlayer.camData);
    debug_line(glm::vec3(1,0,1), glm::vec3(0,0,1), lineColor, gEditorPlayer.camData);
    debug_line(glm::vec3(0,0,1), glm::vec3(0,0,0), lineColor, gEditorPlayer.camData);

    EditorPlayer_UpdateMovementDirection(&context);
    EditorPlayer_UpdatePosition(&context);    
}

GAMESTATE_RENDER(LevelEditor)
{
    G_RenderLevelEditor();
}

GAMESTATE_POSTRENDER(LevelEditor)
{
}

GAMESTATE_EDITOR(LevelEditor)
{}

GAMESTATE_DESTROY(LevelEditor)
{}

EXPORT_GAME_STATE(LevelEditor, gStateLevelEditor);
