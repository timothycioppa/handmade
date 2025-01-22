#include "ui_button.hpp"
#include "../r_main.hpp"


bool cursor_over_button(glm::vec2 mousePos, button & b)
{
     return (mousePos.x > b.lowerLeft.x && 
            mousePos.y > b.lowerLeft.y &&
            mousePos.x < b.lowerLeft.x + b.dimensions.x &&
            mousePos.y < b.lowerLeft.y + b.dimensions.y);
}

void render_button(button & b, float windowWidth, float windowHeight)
{
    glm::vec3 screenPos = b.lowerLeft;

    screenPos.x /= windowWidth;
    screenPos.y /= windowHeight;
    screenPos -= 0.5f;
    screenPos *= 2.0f;
    screenPos.z = 0;
   
    glm::vec3 dims = b.dimensions;
    dims *= 2.0f;
    
    dims.x /= windowWidth;
    dims.y /= windowHeight;
    dims.z = 0;

    R_DrawColoredRect(screenPos, dims, b.color);

    float xOffset = 10.0f;
    float yOffset = 10.0f;
    R_DrawText(b.text, b.lowerLeft.x +xOffset, b.lowerLeft.y + yOffset, .5, glm::vec3(0.0f), GameFont::Ariel);
}


void update_button_state(button & butt, glm::vec2 mousePos, bool mousePressed, bool mouseReleased) 
{
    // mouse is over the button
    if (cursor_over_button(mousePos, butt))
    {

        if (!butt.hovered) 
        {
            butt.hovered = true;
            butt.onMouseEnter();
        }

        // press button, now down
        if (mousePressed)
        {
            if (!butt.isDown) 
            {
                butt.onPressedDown();
            }

            butt.isDown = true;
            butt.color = butt.PressedColor;
        }

        // release button over mouse
        if (mouseReleased) 
        {
            bool wasDown = butt.isDown;
            butt.isDown = false;

            if (wasDown) 
            {
                butt.onClick();
            }
        }

        if (!butt.isDown)
        {
            butt.color = butt.HoverColor;
        }
    } 
    else 
    {       
        if (butt.hovered) 
        {
            butt.onMouseExit();
        }

        butt.hovered = false;

        if (!butt.isDown)
        {
            butt.color = butt.DefaultColor;
        }

        if (mouseReleased) 
        {
            butt.isDown = false;
        }
    }
}

void button_noop(void) {}
