#include "GAME_main.hpp"

#define MOUSE_MOVE_THRESHOLD 0.001f

void process_keyboard_event(system_event * evt, game_context & context) ;
void process_mouse_button_event(system_event * evt, game_context & context) ;
void process_mouse_move(system_event * evt, game_context & context) ;

game_context gContext;
glm::vec2 mousePos;
glm::vec2 mouseDelta;

void GAME_Initialize() 
{ 
	R_Init(); 		// init rendering system
	G_Init(); 		// intit game graphics	

	gContext.gameRunning = true; 
	gContext.deltaTime = 0.0f;
	gContext.applicationTime = 0.0f;
	gContext.sinTime = (float) sin(gContext.applicationTime);
	gContext.cosTime = (float) cos(gContext.applicationTime);

	register_game_state(GameState::MAIN,  &gMainState);

	gCurrentGameState = &gMainState;
	gCurrentGameState->isRunning = true;
	gCurrentGameState->Init(gContext);
}

void GAME_ProcessFrame(game_context & context) 
{ 
    // handle all queued events
    system_event * evt = nullptr;

	while ((evt = pop_event()) != nullptr) 
	{ 
		GAME_ProcessEvent(evt, context);
	}	
		
    // run the game state machine
	if (!(gCurrentGameState->isRunning))
	{ 
		gCurrentGameState->Destroy(context);
    	gCurrentGameState = StateMap[gCurrentGameState->nextState];		
        gCurrentGameState->Init(context);
		gCurrentGameState->isRunning = true;
	}
    
	gCurrentGameState->Update(context);	
    gCurrentGameState->Render(context);
}

void GAME_ProcessEvent(system_event * evt, game_context & context) 
{ 
	context.movingMouse = false;
	
	switch (evt->Type) 
	{ 
		case event_type::KEY_EVENT: 
		{
            process_keyboard_event(evt, context);
		} break;
		
		case event_type::MOUSE_BUTTON_EVENT: 
		{
			process_mouse_button_event(evt, context);
		} break;
		
		case event_type::MOUSE_MOVEMENT_EVENT: 
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
	G_Cleanup();
	R_Cleanup();
}

/* helper functions */
void process_keyboard_event(system_event * evt, game_context & context) 
{ 
	KeyCodes key = KeyCodes (evt->arg1);			
    ButtonActions action = ButtonActions(evt->arg2);
    InputModifiers mod = InputModifiers(evt->arg3);

    if (action == ButtonActions::A_PRESSED) 
    { 
        switch(key) 
        { 
            case KeyCodes::KEY_S: { context.left = true;} break;
            case KeyCodes::KEY_D: { context.back = true;} break;
            case KeyCodes::KEY_F: { context.right = true;} break;
            case KeyCodes::KEY_E: { context.forward = true;} break;
        }		
    }

    if (action == ButtonActions::A_RELEASED) 
    { 
        switch(key) 
        { 
            case KeyCodes::KEY_S: { context.left = false;} break;
            case KeyCodes::KEY_D: { context.back = false;} break;
            case KeyCodes::KEY_F: { context.right = false;} break;
            case KeyCodes::KEY_E: { context.forward = false;} break;
            case KeyCodes::KEY_ESCAPE: { context.gameRunning = false; } break;
        }		
    }
}

void process_mouse_button_event(system_event * evt, game_context & context) 
{ 
    MouseButtons button = MouseButtons (evt->arg1);			
    ButtonActions action = ButtonActions(evt->arg2);
    InputModifiers mod = InputModifiers(evt->arg3);

    if (button == MouseButtons::M_RIGHT) 
    { 
        if (action == ButtonActions::A_PRESSED) 
        { 
            context.rightMouseHeld = true;
        }	
        if (action == ButtonActions::A_RELEASED) 
        { 
            context.rightMouseHeld = false;
        }		
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
