#ifndef _GAMESTATE_HPP
#define _GAMESTATE_HPP

#include "game_context.hpp"
#include <map>

typedef void (*GameStateFuntion) (game_context & context);

enum GameState 
{ 
	STARTUP,
	MAIN,
	NONE
};

struct game_state 
{ 
	GameStateFuntion Init;
	GameStateFuntion Update;
	GameStateFuntion Render;
	GameStateFuntion Destroy;
	bool isRunning;
	GameState nextState;
};

extern game_state * gCurrentGameState;
extern std::map<GameState, game_state*> StateMap;
extern game_state MainState;

void register_game_state(GameState, game_state*);
void request_state_change(GameState newState);

#endif