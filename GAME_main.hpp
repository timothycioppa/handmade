#ifndef _GAME_MAIN_HPP
#define _GAME_MAIN_HPP

#include "i_input.hpp"
#include "game_context.hpp"
#include "game_state.hpp"
#include "g_main.hpp"
#include "r_main.hpp"
#include "player.hpp"
#include "GameStates/Gameplay/Gameplay.hpp"
#include "game_data.hpp"

extern glm::vec2 mousePos;
extern glm::vec2 mouseDelta;
extern game_state MainState;

void GAME_Initialize() ;
void GAME_ProcessEvent(system_event * evt, game_context & context) ;
void GAME_ProcessFrame(game_context & context) ;
void GAME_PostProcessFrame(game_context & context, float frameTime) ;
void GAME_Cleanup();

#endif