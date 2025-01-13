#ifndef _JC_MAIN_HPP
#define _JC_MAIN_HPP
#include "GAME_main.hpp"
#include "platform_common.hpp"
#include "Graphics/GLTextRenderer.hpp"
#include "Transform.hpp"
#include "Graphics/GLMesh.hpp"
#include "Graphics/GLTexture.hpp"
#include "Graphics/GLTextRenderer.hpp"
#include "game_data.hpp"
#include "edtitor_main.hpp"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#endif