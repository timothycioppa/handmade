#ifndef _MAIN_STATE_HPP
#define _MAIN_STATE_HPP

#include "../game_state.hpp"
#include "../game_context.hpp"
#include "../player.hpp"
#include "../g_main.hpp"

extern game_state gStateGameplay;

GAMESTATE_INIT(Gameplay);
GAMESTATE_UPDATE(Gameplay);
GAMESTATE_RENDER(Gameplay);
GAMESTATE_POSTRENDER(Gameplay);
GAMESTATE_EDITOR(Gameplay);
GAMESTATE_DESTROY(Gameplay);

#endif