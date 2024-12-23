#include <stdio.h>
#include <stdlib.h>
#include "main.hpp"

int main(int argc, char** argv)
{
	GLFWwindow* window = CreateWindow();

	if (window == NULL) 
	{ 
		glfwTerminate();
		return -1;
	}
	
	GAME_Initialize();

	double lastUpdateTime = glfwGetTime();

	while (gContext.gameRunning)
	{
		float preTickTime = glfwGetTime();
		GAME_ProcessFrame(gContext);	
		double postTickTime = glfwGetTime();

		GAME_PostProcessFrame(gContext, float(postTickTime - lastUpdateTime));	
		lastUpdateTime = postTickTime;		
	
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwWindowShouldClose(window) != 0) 
		{ 
			gContext.gameRunning = false;
		}
	} 

	GAME_Cleanup() ;
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

GLFWwindow * CreateWindow() 
{ 
	GLFWwindow* window;

	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Rjune", NULL, NULL);

	if (window == NULL)
	{
		return NULL;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	gladLoadGL();

	return window;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	system_event evt;
	evt.Type = event_type::MOUSE_BUTTON_EVENT;
	evt.arg1 = int (translate_mouse_button(button));
	evt.arg2 = int (translate_input_action(action));
	evt.arg3 = int (translate_input_modifier(mods));
	push_event(&evt);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{		
	system_event evt;
	evt.Type = event_type::KEY_EVENT;
	evt.arg1 = int (translate_key_code(key));
	evt.arg2 = int (translate_input_action(action));
	evt.arg3 = int (translate_input_modifier(mods));
	push_event(&evt);
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	static float xPos = xpos;
	static float yPos = ypos;
	mousePos = { float(xpos), float(ypos)};
	mouseDelta = { float(xpos - xPos) / float(WINDOW_WIDTH), float(-(ypos - yPos)) / float(WINDOW_HEIGHT) };
	system_event evt;
	evt.Type = event_type::MOUSE_MOVEMENT_EVENT;
	push_event(&evt);
	xPos = xpos;
	yPos = ypos;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}