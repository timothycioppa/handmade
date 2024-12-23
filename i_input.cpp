#include "i_input.hpp"

#define MAXEVENTS 64
system_event eventQueue[MAXEVENTS];
unsigned int head = 0;
unsigned int tail = 0;


void push_event(system_event * evt)
{
    eventQueue[head] = *evt;            
    head = (head + 1) % MAXEVENTS;
}

system_event *pop_event(void)
{
    system_event *result = nullptr;

    if (head != tail)
    {
        result = &eventQueue[tail];
        tail = (tail + 1) % MAXEVENTS;
    }
    return result;
}

KeyCodes translate_key_code(int glfw_key)  
{
    switch(glfw_key) 
    {
        case GLFW_KEY_S: { return KeyCodes::KEY_S; } break;
        case GLFW_KEY_D: { return KeyCodes::KEY_D; } break;
        case GLFW_KEY_F: { return KeyCodes::KEY_F; } break;
        case GLFW_KEY_E: { return KeyCodes::KEY_E; } break;
        case GLFW_KEY_ESCAPE: { return KeyCodes::KEY_ESCAPE; } break;
        default: { return KeyCodes::KEY_UNKNOWN; } break;
    }
}

MouseButtons translate_mouse_button(int glfw_button) {
  switch(glfw_button) 
    {
        case GLFW_MOUSE_BUTTON_LEFT: { return MouseButtons::M_LEFT; } break;
        case GLFW_MOUSE_BUTTON_MIDDLE: { return MouseButtons::M_MIDDLE; } break;
        case GLFW_MOUSE_BUTTON_RIGHT: { return MouseButtons::M_RIGHT; } break;
        default: { return MouseButtons::MOUSE_BUTTON_UNKNOWN; } break;
    }
}


ButtonActions translate_input_action(int glfw_key)  
{
    switch (glfw_key) 
    { 
        case GLFW_PRESS: { return ButtonActions::A_PRESSED; } break;
        case GLFW_REPEAT: { return ButtonActions::A_HELD; } break;
        case GLFW_RELEASE: { return ButtonActions::A_RELEASED; } break;
        default: { return ButtonActions::ACTION_UNKNOWN; } break;
    }
}

InputModifiers translate_input_modifier(int glfw_mod)  {
    switch (glfw_mod) 
    { 
        case GLFW_MOD_SHIFT: { return InputModifiers::M_SHIFT; } break;
        case GLFW_MOD_CONTROL: { return InputModifiers::M_CONTROL; } break;
        case GLFW_MOD_ALT: { return InputModifiers::M_ALT; } break;
        default: { return InputModifiers::MOD_UNKNOWN; } break;
    }
}