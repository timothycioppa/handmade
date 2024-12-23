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
    KEY_UNKNOWN = 5
};

enum MouseButtons
{ 
    M_LEFT = 0, 
    M_MIDDLE = 1, 
    M_RIGHT = 2, 
    MOUSE_BUTTON_UNKNOWN = 3
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

enum event_type 
{ 
    KEY_EVENT,
    MOUSE_BUTTON_EVENT,
    MOUSE_MOVEMENT_EVENT 
};

struct system_event 
{ 
    event_type Type;
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