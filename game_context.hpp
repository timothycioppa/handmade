#ifndef _GAME_CONTEXT_HPP
#define _GAME_CONTEXT_HPP

#include "platform_common.hpp"

struct game_context 
{ 
	bool movingMouse;
	bool rightMouseHeld;
	bool leftMouseHeld;
	bool forward;
	bool back;
	bool left;
	bool right;
	glm::vec2 mouseDelta;
	glm::vec2 mousePosition;

	float deltaTime;
	float applicationTime;
	float sinTime;
	float cosTime;
	bool gameRunning;
};

extern game_context gContext;

#endif