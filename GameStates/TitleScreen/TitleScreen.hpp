#ifndef _TITLE_SCREEN_HPP
#define _TITLE_SCREEN_HPP

#include "../game_state.hpp"
#include "../game_context.hpp"

extern game_state gTitleScreenState;

GAMESTATE_INIT(TitleScreen);
GAMESTATE_UPDATE(TitleScreen);
GAMESTATE_RENDER(TitleScreen);
GAMESTATE_POSTRENDER(TitleScreen);
GAMESTATE_EDITOR(TitleScreen);
GAMESTATE_DESTROY(TitleScreen);

#endif