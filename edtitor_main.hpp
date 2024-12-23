#ifndef _EDITOR_MAIN_HPP
#define _EDITOR_MAIN_HPP

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "platform_common.hpp"


void Editor_Init(GLFWwindow* window);
void Editor_BeginFrame() ;
void Editor_RenderFrame();
void Editor_EndFrame() ;
void Editor_Shutdown() ;

#endif