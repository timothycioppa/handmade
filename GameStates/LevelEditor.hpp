#ifndef _LEVEL_EDITOR_HPP
#define _LEVEL_EDITOR_HPP

#include "../game_state.hpp"
#include "../game_context.hpp"
#include "../g_main.hpp"
#include "../editor_controller.hpp"

extern game_state gStateLevelEditor;

GAMESTATE_INIT(LevelEditor);
GAMESTATE_UPDATE(LevelEditor);
GAMESTATE_RENDER(LevelEditor);
GAMESTATE_POSTRENDER(LevelEditor);
GAMESTATE_EDITOR(LevelEditor);
GAMESTATE_DESTROY(LevelEditor);

#endif