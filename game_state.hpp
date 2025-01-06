#ifndef _GAMESTATE_HPP
#define _GAMESTATE_HPP

#include "game_context.hpp"
#include <map>

typedef void (*GameStateFuntion) (game_context & context);

enum GameState 
{ 
	STARTUP,
	GAMEPLAY,
	LEVEL_EDITOR,
	NONE
};

struct game_state 
{ 
	GameStateFuntion Init;
	GameStateFuntion Update;
	GameStateFuntion Render;
	GameStateFuntion PostRender;
	GameStateFuntion Editor;
	GameStateFuntion Destroy;
	bool isRunning;
	GameState nextState;
};

extern game_state * gCurrentGameState;
extern std::map<GameState, game_state*> StateMap;
extern game_state MainState;

void register_game_state(GameState, game_state*);
void request_state_change(GameState newState);
void set_initial_state(GameState state) ;
void check_for_state_change(game_context & context);


#define EXPORT_GAME_STATE_FULL(name, varname, nextState) game_state varname = \
{ \
    name##_Init,\
    name##_Update,\
    name##_Render,\
    name##_PostRender,\
    name##_Editor,\
    name##_Destroy,\
    false,       \
    nextState \
} \

#define EXPORT_GAME_STATE(name, varname) EXPORT_GAME_STATE_FULL(name, varname, GameState::NONE)

#define GAMESTATE_INIT(name) void name##_Init(game_context & context)
#define GAMESTATE_UPDATE(name) void name##_Update(game_context & context)
#define GAMESTATE_RENDER(name) void name##_Render(game_context & context)
#define GAMESTATE_POSTRENDER(name) void name##_PostRender(game_context & context)
#define GAMESTATE_EDITOR(name) void name##_Editor(game_context & context)
#define GAMESTATE_DESTROY(name) void name##_Destroy(game_context & context)

#endif