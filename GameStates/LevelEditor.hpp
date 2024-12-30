#ifndef _LEVEL_EDITOR_HPP
#define _LEVEL_EDITOR_HPP

#include "../game_state.hpp"
#include "../game_context.hpp"
#include "../g_main.hpp"
#include "../editor_controller.hpp"


extern game_state gStateLevelEditor;

void LevelEditor_Init(game_context & context);
void LevelEditor_Update(game_context & context);
void LevelEditor_Render(game_context & context);
void LevelEditor_Editor(game_context & context);
void LevelEditor_Destroy(game_context & context);

#endif