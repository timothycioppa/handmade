
#include <stdio.h>
#include <stdlib.h>
#include "main.hpp"

#define TARGET_FPS 90.0f
#define LIMIT_FRAMERATE 0
#define WAIT_UNTIL(time) while (glfwGetTime() < time) {}

int main(int argc, char** argv)
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
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	gladLoadGL();


	#ifdef EDITOR_DEBUG
		Editor_Init(window);
	#endif	

	GAME_Initialize();

	double lastUpdateTime = glfwGetTime();

	#if LIMIT_FRAMERATE == 1
		float targetFrameTime = 1.0f / TARGET_FPS;
	#endif


	while (gContext.gameRunning)
	{

		#ifdef EDITOR_DEBUG
				Editor_BeginFrame();
		#endif

		double preTickTime = glfwGetTime();
		GAME_ProcessFrame(gContext);	
		double postTickTime = glfwGetTime();

		GAME_PostProcessFrame(gContext, float(postTickTime - lastUpdateTime));	
		
		lastUpdateTime = postTickTime;		

		#ifdef EDITOR_DEBUG	
				Editor_RenderFrame(gContext);
				Editor_EndFrame();
		#endif

		#if LIMIT_FRAMERATE == 1
				double frameDuration = glfwGetTime() - preTickTime;
				if (frameDuration < targetFrameTime) 
				{
					double remaining = targetFrameTime - frameDuration;
					double targetTime = glfwGetTime() + remaining;
					WAIT_UNTIL(targetTime)
				}
		#endif


		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwWindowShouldClose(window) != 0) 
		{ 
			gContext.gameRunning = false;
		}
	} 
	
	#ifdef EDITOR_DEBUG
		Editor_Shutdown();
	#endif
	


	GAME_Cleanup() ;
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	system_event evt;
	evt.Type = system_event_type::MOUSE_BUTTON_EVT;
	evt.arg1 = int (translate_mouse_button(button));
	evt.arg2 = int (translate_input_action(action));
	evt.arg3 = int (translate_input_modifier(mods));
	push_event(&evt);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{			
	system_event evt;
	evt.Type = system_event_type::KEY_EVT;
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
	evt.Type = system_event_type::MOUSE_MOVEMENT_EVT;
	push_event(&evt);
	xPos = xpos;
	yPos = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	gContext.windowWidth = width;
	gContext.windowHeight = height;
	gContext.aspectRatio = width / height;
	
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}