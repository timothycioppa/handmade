#include "game_context.hpp"

// global game context
game_context gContext;

bool key_pressed(KeyCodes key) 
{ 
    return gContext.key_states[int(key)].pressed;
}

bool key_held(KeyCodes key) { 
    return gContext.key_states[int(key)].down;
}

bool key_released(KeyCodes key) { 
    return gContext.key_states[int(key)].released;
}

bool mouse_button_pressed(MouseButtons button) {
    switch (button) 
    {
        case MouseButtons::M_LEFT: { return gContext.left.pressed; }
        case MouseButtons::M_RIGHT: { return gContext.right.pressed; }
        case MouseButtons::M_MIDDLE: { return gContext.middle.pressed; }
    } 
    return false;
}

bool mouse_button_held(MouseButtons button) {
  switch (button) 
    {
        case MouseButtons::M_LEFT: { return gContext.left.down; }
        case MouseButtons::M_RIGHT: { return gContext.right.down; }
        case MouseButtons::M_MIDDLE: { return gContext.middle.down; }
    }
    return false;
}

bool mouse_button_released(MouseButtons button) {
    switch (button) 
    {
        case MouseButtons::M_LEFT: { return gContext.left.released; }
        case MouseButtons::M_RIGHT: { return gContext.right.released; }
        case MouseButtons::M_MIDDLE: { return gContext.middle.released; }
    }
    return false;
}