#ifndef _UI_BUTTON_HPP
#define _UI_BUTTON_HPP

#include "../platform_common.hpp"

typedef void (*ButtonCallback)(void);

struct button 
{ 
    glm::vec3 lowerLeft;
    glm::vec3 dimensions;
    glm::vec3 color;
    glm::vec3 HoverColor;
    glm::vec3 DefaultColor;
    glm::vec3 PressedColor;
    const char* text;
    bool isDown;
    bool hovered;
    ButtonCallback onClick;
    ButtonCallback onPressedDown;
    ButtonCallback onMouseEnter;
    ButtonCallback onMouseExit;
};



void render_button(button & b, float windowWidth, float windowHeight);
void update_button_state(button & butt, glm::vec2 mousePos, bool mousePressed, bool mouseReleased) ;
void button_noop(void);

#define BUTTON_STUB button_noop

#endif