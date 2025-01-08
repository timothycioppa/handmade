#ifndef _GAME_CONTEXT_HPP
#define _GAME_CONTEXT_HPP

#include "i_input.hpp"

struct game_context 
{ 
	bool movingMouse;
	glm::vec2 mouseDelta;
	glm::vec2 mousePosition;

	mouse_button_state left;
	mouse_button_state middle;
	mouse_button_state right;	
	key_state key_states[NUM_KEY_CODES];

	float deltaTime;
	float applicationTime;
	float sinTime;
	float cosTime;
	bool gameRunning;

	float windowWidth;
	float windowHeight;
	float aspectRatio;
};

extern game_context gContext;

bool key_pressed(KeyCodes);
bool key_held(KeyCodes);
bool key_released(KeyCodes);

bool mouse_button_pressed(MouseButtons);
bool mouse_button_held(MouseButtons);
bool mouse_button_released(MouseButtons);

#endif