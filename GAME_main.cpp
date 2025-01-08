#include "GAME_main.hpp"
#include "GameStates/LevelEditor/LevelEditor.hpp"
#include "game_audio.hpp"

#define MOUSE_MOVE_THRESHOLD 0.001f

void process_keyboard_event(system_event * evt, game_context & context) ;
void process_mouse_button_event(system_event * evt, game_context & context) ;
void process_mouse_move(system_event * evt, game_context & context) ;

glm::vec2 mousePos;
glm::vec2 mouseDelta;

void GAME_Initialize() 
{ 
	R_Init(); 		// init rendering system
	G_Init(); 		// intit game graphics	
    Audio_Init();
   
	gContext.gameRunning = true; 
	gContext.deltaTime = 0.0f;
	gContext.applicationTime = 0.0f;
	gContext.sinTime = (float) sin(gContext.applicationTime);
	gContext.cosTime = (float) cos(gContext.applicationTime);
    
    gContext.windowWidth = WINDOW_WIDTH;
    gContext.windowHeight = WINDOW_HEIGHT;
    gContext.aspectRatio = float(WINDOW_WIDTH) / float(WINDOW_HEIGHT);
    
    for (int i = 0; i < NUM_KEY_CODES; i++) 
    { 
        key_state & state = gContext.key_states[i];
        state.pressed = false;
        state.down = false;
        state.released = false;
    }

	register_game_state(GameState::GAMEPLAY,  &gStateGameplay);
	register_game_state(GameState::LEVEL_EDITOR,  &gStateLevelEditor);
    set_initial_state(GameState::GAMEPLAY);
}

#define PROCESS_MOUSE_HELD(button) if ((button).pressed == true) { (button).pressed = false; (button).down = true; }

// a single frame tick of the game
void GAME_ProcessFrame(game_context & context) 
{ 
    // make sure if we pressed last frame, we clear it and set the down flag
    PROCESS_MOUSE_HELD(context.left)
    PROCESS_MOUSE_HELD(context.middle)
    PROCESS_MOUSE_HELD(context.right)

    // make sure if we released last frame, we clear that flag
    context.left.released = false;
    context.right.released = false;
    context.middle.released = false;

    // handle all queued events
    system_event * evt = nullptr;

	while ((evt = pop_event()) != nullptr) 
	{ 
		GAME_ProcessEvent(evt, context);
	}	
		
    check_for_state_change(context);
    
	gCurrentGameState->Update(context);	
    gCurrentGameState->Render(context);
    gCurrentGameState->PostRender(context);

    #ifdef EDITOR_DEBUG
        gCurrentGameState->Editor(context);
    #endif

    Audio_PlayAllSounds();
}

void GAME_ProcessEvent(system_event * evt, game_context & context) 
{ 
	context.movingMouse = false;
	
	switch (evt->Type) 
	{ 
		case system_event_type::KEY_EVT: 
		{
            process_keyboard_event(evt, context);
		} break;
		
		case system_event_type::MOUSE_BUTTON_EVT: 
		{
			process_mouse_button_event(evt, context);
		} break;
		
		case system_event_type::MOUSE_MOVEMENT_EVT: 
		{
            process_mouse_move(evt, context);
		} break;

		default: {} break;
	}
}

void GAME_PostProcessFrame(game_context & context, float frameTime) 
{ 
	gContext.deltaTime = frameTime;
	gContext.applicationTime += frameTime;
	gContext.sinTime = (float) sin(gContext.applicationTime);
	gContext.cosTime = (float) cos(gContext.applicationTime);
}

void GAME_Cleanup()
{ 
    Audio_Shutdown();
	G_Cleanup();
	R_Cleanup();
}

void process_keyboard_event(system_event * evt, game_context & context) 
{ 
	KeyCodes key = KeyCodes (evt->arg1);			
    ButtonActions action = ButtonActions(evt->arg2);
    InputModifiers mod = InputModifiers(evt->arg3);
   
    for (int i = 0; i < NUM_KEY_CODES; i++) 
    { 
        context.key_states[i].pressed = false;
        context.key_states[i].released = false;
    }

    if (key != KeyCodes::KEY_UNKNOWN) 
    {
       key_state & state = context.key_states[int(key)];
        
       switch (action) 
       { 
          case ButtonActions::A_PRESSED:  { state.pressed = true; state.down = true; } break;
          case ButtonActions::A_RELEASED: { state.released = true; state.down = false;} break;
       }
    }
}

void process_mouse_button_event(system_event * evt, game_context & context) 
{ 
   
    MouseButtons button = MouseButtons (evt->arg1);			
    ButtonActions action = ButtonActions(evt->arg2);
    InputModifiers mod = InputModifiers(evt->arg3);
   
    context.left.pressed = false;
    context.right.pressed = false;
    context.middle.pressed = false;
    context.left.released = false;
    context.right.released = false;
    context.middle.released = false;

    switch (button) {
        case MouseButtons::M_LEFT: 
        { 
            if (action == ButtonActions::A_PRESSED) { gContext.left.pressed = true;  } 
            if (action == ButtonActions::A_RELEASED) { gContext.left.released = true; gContext.left.down = false; }   
        }break;

        case MouseButtons::M_MIDDLE: 
        { 
            if (action == ButtonActions::A_PRESSED) { gContext.middle.pressed = true;  } 
            if (action == ButtonActions::A_RELEASED) { gContext.middle.released = true; gContext.middle.down = false; }   
        }break;

          case MouseButtons::M_RIGHT: 
        { 
            if (action == ButtonActions::A_PRESSED) { gContext.right.pressed = true; } 
            if (action == ButtonActions::A_RELEASED) { gContext.right.released = true; gContext.right.down = false; }    
        }
        break;
    } 
}

void process_mouse_move(system_event * evt, game_context & context) 
{ 
    float x = mouseDelta.x;
    float y = mouseDelta.y;
    float sqrtDist = 1000.0f * (x * x + y * y);

    if (sqrtDist > MOUSE_MOVE_THRESHOLD) 
    {                     
        context.movingMouse = true;
    }

    context.mouseDelta = mouseDelta;
    context.mousePosition = mousePos;
}
