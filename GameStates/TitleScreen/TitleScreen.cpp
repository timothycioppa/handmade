#include "TitleScreen.hpp"
#include "../../g_main.hpp"
#include "../../r_main.hpp"
#include "../../game_audio.hpp"
#include "../../ui/ui_button.hpp"
#include "../../imgui/imgui.h"
#include "../../game_data.hpp"

struct title_screen_context 
{
    button gameplayButton;
    button editorButton;
};

title_screen_context gTitleContext;

void wait(float time);
void on_mouse_over(void);
void on_mouse_exit(void);
void gotoGameplay(void);
void gotoEditor(void);
void init_UI(void);

GAMESTATE_INIT(TitleScreen)
{
    init_UI();
}

GAMESTATE_UPDATE(TitleScreen)
{  
    glm::vec2 mousePos = gContext.mousePosition;
    mousePos.y = gContext.windowHeight - mousePos.y;
    bool mousePressed = mouse_button_pressed(MouseButtons::M_LEFT);
    bool mouseReleased = mouse_button_released(MouseButtons::M_LEFT);    
    update_button_state(gTitleContext.gameplayButton, mousePos, mousePressed, mouseReleased);
    update_button_state(gTitleContext.editorButton, mousePos, mousePressed, mouseReleased);
}

GAMESTATE_RENDER(TitleScreen)
{
    render_button(gTitleContext.gameplayButton, WINDOW_WIDTH, WINDOW_HEIGHT);
    render_button(gTitleContext.editorButton, WINDOW_WIDTH, WINDOW_HEIGHT);
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


void wait(float time)
{
    float current = glfwGetTime();
    while (glfwGetTime() < current + time) {}
}

void gotoGameplay() 
{ 
    wait(0.25f);
    request_state_change(GameState::GAMEPLAY);
}

void gotoEditor() 
{ 
    wait(0.25f);
    request_state_change(GameState::LEVEL_EDITOR);
}

void on_mouse_over(void)
{
    Audio_PlaySound(SoundCode::BUTTON_HOVER);
}

void on_mouse_exit(void)
{
    Audio_PlaySound(SoundCode::BUTTON_HOVER);
}

void init_UI(void)
{
    gTitleContext.gameplayButton.lowerLeft = glm::vec3(50, 800, 1);
    gTitleContext.gameplayButton.dimensions = glm::vec3(100, 50, 1);
    gTitleContext.gameplayButton.color = glm::vec3(1.0f);
    gTitleContext.gameplayButton.DefaultColor = glm::vec3(1.0f);
    gTitleContext.gameplayButton.HoverColor = glm::vec3(0.2f);
    gTitleContext.gameplayButton.PressedColor = glm::vec3(0.6f);
    gTitleContext.gameplayButton.hovered = false;
    gTitleContext.gameplayButton.isDown = false;
    gTitleContext.gameplayButton.text = "GAME";
    gTitleContext.gameplayButton.onClick = gotoGameplay;
    gTitleContext.gameplayButton.onMouseEnter = on_mouse_over;
    gTitleContext.gameplayButton.onMouseExit = on_mouse_exit;
    gTitleContext.gameplayButton.onPressedDown = BUTTON_STUB;

    gTitleContext.editorButton.lowerLeft = glm::vec3(350, 800, 1);
    gTitleContext.editorButton.dimensions =  glm::vec3(100, 50, 1);
    gTitleContext.editorButton.color = glm::vec3(1.0f);
    gTitleContext.editorButton.DefaultColor = glm::vec3(1.0f);
    gTitleContext.editorButton.HoverColor = glm::vec3(0.2f);
    gTitleContext.editorButton.PressedColor = glm::vec3(0.6f);
    gTitleContext.editorButton.hovered = false;
    gTitleContext.editorButton.isDown = false;
    gTitleContext.editorButton.text = "EDITOR";
    gTitleContext.editorButton.onClick = gotoEditor;
    gTitleContext.editorButton.onMouseEnter = on_mouse_over;
    gTitleContext.editorButton.onMouseExit = on_mouse_exit;
    gTitleContext.editorButton.onPressedDown = BUTTON_STUB;
}

EXPORT_GAME_STATE(TitleScreen, gTitleScreenState);
