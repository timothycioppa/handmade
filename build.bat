@set IMGUI=imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_impl_glfw.cpp imgui/imgui_impl_opengl3.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui.cpp 
@set SOURCES=main.cpp glad.c  light.cpp i_input.cpp scene_object.cpp math_utils.cpp scene_parser.cpp GameStates/MainState.cpp Graphics/GLShader.cpp  Graphics/GLTextRenderer.cpp Graphics/GLMesh.cpp Graphics/GLTexture.cpp player.cpp GAME_main.cpp game_state.cpp g_main.cpp r_main.cpp utils.cpp 
@set INCLUDES=-Iinclude -Iinclude/GL -Iinclude/glad -Iinlude/glfw -Iinclude/glm -Iinclude/freetype
@set LIBS= freetype.dll libglfw3.a libreactphysics3d.a -lopengl32 -lgdi32 -lkernel32 -luser32 -lws2_32
@set OUTPUT=main
@set FLAGS=
g++ %FLAGS% %SOURCES% -o %OUTPUT% %INCLUDES% %LIBS%