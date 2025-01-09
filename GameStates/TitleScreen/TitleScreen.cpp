#include "TitleScreen.hpp"
#include "../../g_main.hpp"
#include "../../r_main.hpp"
#include "../../game_audio.hpp"
#define COLOR_HOVER glm::vec3(0,0.5f, 0)

void wait(float time) 
{
    float current = glfwGetTime();
    while (glfwGetTime() < current + time) {}
}

typedef void (*ButtonCallback)(void);

struct button 
{ 
    glm::vec3 lowerLeft;
    glm::vec3 dimensions;
    glm::vec3 color;
    const char* text;
    bool isDown;
    bool hovered;
    ButtonCallback callback;
};

void gotoGameplay() 
{ 
    request_state_change(GameState::GAMEPLAY);
}

void gotoEditor() 
{ 
    request_state_change(GameState::LEVEL_EDITOR);
}

void draw_button(button & b) 
{
    glm::vec3 screenPos = b.lowerLeft;

    screenPos.x /= gContext.windowWidth;
    screenPos.y /= gContext.windowHeight;
    screenPos -= 0.5f;
    screenPos *= 2.0f;
    screenPos.z = 0;
   
    glm::vec3 dims = b.dimensions;
    dims.x /= gContext.windowWidth;
    dims.y /= gContext.windowHeight;
    dims.z = 0;

    R_DrawColoredRect(screenPos, dims, b.color);
    R_DrawText(b.text, b.lowerLeft.x + 10, b.lowerLeft.y + 10, .5, glm::vec3(0.0f), GameFont::Ariel);
}

button gameplay, editor;

GAMESTATE_INIT(TitleScreen)
{
    gameplay.lowerLeft = glm::vec3(50, 900, 1);
    gameplay.dimensions = glm::vec3(200, 100, 1);
    gameplay.color = glm::vec3(1.0f);
    gameplay.text = "GAME";
    gameplay.callback = gotoGameplay;


    editor.lowerLeft = glm::vec3(250, 900, 1);
    editor.dimensions = glm::vec3(220, 100, 1);
    editor.color = glm::vec3(1.0f);
    editor.text = "EDITOR";
    editor.callback = gotoEditor;

}

bool mouseHover(glm::vec2 mousePos, button & b) 
{
    return (mousePos.x > b.lowerLeft.x && 
            mousePos.y > b.lowerLeft.y &&
            mousePos.x < b.lowerLeft.x + b.dimensions.x &&
            mousePos.y < b.lowerLeft.y + b.dimensions.y);
}

void processButton(button & butt, glm::vec2 mousePos, bool mousePressed, bool mouseReleased) 
{
    // mouse is over the button
    if (mouseHover(mousePos, butt))
    {
        if (!butt.hovered) 
        {
            butt.hovered = true;
            Audio_PlaySound(SoundCode::BUTTON_HOVER);
        }

        // press button, now down
        if (mousePressed)
        {
            if (!butt.isDown) 
            {
                Audio_PlaySound(SoundCode::BUTTON_PRESSED);
            }
            butt.isDown = true;
            butt.color = glm::vec3(1,0,0);
        }

        // release button over mouse
        if (mouseReleased) 
        {
            bool wasDown = butt.isDown;
            butt.isDown = false;

            if (wasDown) 
            {
                butt.callback();
            }
        }

        if (!butt.isDown)
        {
            butt.color = COLOR_HOVER;
        }

    } else 
    {       

        butt.hovered = false;

        if (!butt.isDown)
        {
            butt.color = glm::vec3(1, 1, 1);
        }

        if (mouseReleased) 
        {
            butt.isDown = false;
        }
    }
}

GAMESTATE_UPDATE(TitleScreen)
{  
    glm::vec2 mousePos = gContext.mousePosition;
    mousePos.y = gContext.windowHeight - mousePos.y;
    bool mousePressed = mouse_button_pressed(MouseButtons::M_LEFT);
    bool mouseReleased = mouse_button_released(MouseButtons::M_LEFT);    
    processButton(gameplay, mousePos, mousePressed, mouseReleased);
    processButton(editor, mousePos, mousePressed, mouseReleased);
}

GAMESTATE_RENDER(TitleScreen)
{
    G_RenderTitleScreen();
    draw_button(gameplay);
    draw_button(editor);
}

GAMESTATE_POSTRENDER(TitleScreen)
{

}

GAMESTATE_EDITOR(TitleScreen)
{
}

GAMESTATE_DESTROY(TitleScreen)
{
}

EXPORT_GAME_STATE(TitleScreen, gTitleScreenState);
