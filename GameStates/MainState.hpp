#ifndef _MAIN_STATE_HPP
#define _MAIN_STATE_HPP

#include "../game_state.hpp"
#include "../game_context.hpp"
#include "../player.hpp"
#include "../g_main.hpp"

extern game_state gMainState;

void Main_Init(game_context & context) ;
void Main_Update(game_context & context) ;
void Main_Render(game_context & context);
void Main_Destroy(game_context & context) ;

#endif