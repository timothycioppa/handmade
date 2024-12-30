#include "game_state.hpp"

std::map<GameState, game_state*> StateMap;
game_state * gCurrentGameState;

void register_game_state(GameState type , game_state* state) 
{
    StateMap.insert({type, state});
}

void request_state_change(GameState newState) 
{     
    gCurrentGameState->isRunning = false;
    gCurrentGameState->nextState = newState;
}

void set_initial_state(GameState state) 
{ 
    gCurrentGameState = StateMap[state];
	gCurrentGameState->isRunning = true;
	gCurrentGameState->Init(gContext);
}

void check_for_state_change(game_context & context)
{
	if (!(gCurrentGameState->isRunning))
	{ 
		gCurrentGameState->Destroy(context);
    	gCurrentGameState = StateMap[gCurrentGameState->nextState];	

        if(gCurrentGameState != nullptr) 
        {
            gCurrentGameState->Init(context);
            gCurrentGameState->isRunning = true;
        }	
	}
}