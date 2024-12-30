#ifndef _MAIN_STATE_HPP
#define _MAIN_STATE_HPP

#include "../game_state.hpp"
#include "../game_context.hpp"
#include "../player.hpp"
#include "../g_main.hpp"

extern game_state gStateGameplay;

void Gameplay_Init(game_context & context) ;
void Gameplay_Update(game_context & context) ;
void Gameplay_Render(game_context & context);
void Gameplay_Editor(game_context & context);
void Gameplay_Destroy(game_context & context) ;

#endif