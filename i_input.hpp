#ifndef _I_INPUT_HPP
#define _I_INPUT_HPP
#include "platform_common.hpp"


enum KeyCodes 
{ 
    KEY_S = 0, 
    KEY_D = 1, 
    KEY_F = 2, 
    KEY_E = 3, 
    KEY_ESCAPE = 4, 
    KEY_UNKNOWN = 5,
    KEY_SPACE = 6
};

#define NUM_KEY_CODES 7

struct key_state 
{ 
    KeyCodes key;
    bool pressed;
    bool down;
    bool released;
};

enum MouseButtons
{ 
    M_LEFT = 0, 
    M_MIDDLE = 1, 
    M_RIGHT = 2, 
    MOUSE_BUTTON_UNKNOWN = 3
};

struct mouse_button_state 
{ 
    MouseButtons button;
    bool pressed;
    bool down;
    bool released;
};

enum ButtonActions 
{ 
    A_PRESSED = 0,
    A_HELD = 1, 
    A_RELEASED = 2,
    ACTION_UNKNOWN = 3
};

enum InputModifiers 
{ 
    M_SHIFT = 0,
    M_ALT = 1,
    M_CONTROL = 2,
    MOD_UNKNOWN = 3
};

enum system_event_type 
{ 
    KEY_EVT = 0,
    MOUSE_BUTTON_EVT = 1,
    MOUSE_MOVEMENT_EVT = 2 
};

struct system_event 
{ 
    system_event_type Type;
    unsigned int arg1, arg2, arg3, arg4, arg5, arg6;
};

extern unsigned int head;
extern unsigned int tail;

void push_event(system_event * evt);
system_event* pop_event();

KeyCodes translate_key_code(int glfw_key)  ;
MouseButtons translate_mouse_button(int glfw_button) ;
ButtonActions translate_input_action(int glfw_key)  ;
InputModifiers translate_input_modifier(int glfw_mod) ;

#endif