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
