INCLUDES=-Iinclude -Iinclude/GL -Iinclude/glad -Iinlude/glfw -Iinclude/glm -Iinclude/freetype
LIBS= freetype.dll glad.a libglfw3.a imgui.a -lopengl32  -lgdi32
RELEASE_FLAGS=-DRELEASE_MODE
DEBUG_FLAGS=-DEDITOR_DEBUG
ALLOBJECTS=build/obj/main.o build/obj/bsp.o build/obj/bsp_collision.o build/obj/editor_controller.o build/obj/editor_main.o build/obj/g_main.o build/obj/game_context.o build/obj/GAME_main.o build/obj/game_state.o build/obj/i_input.o build/obj/light.o build/obj/math_utils.o build/obj/player.o build/obj/r_main.o build/obj/scene_object.o build/obj/scene_parser.o build/obj/utils.o build/obj/GLMesh.o build/obj/GLShader.o build/obj/GLTextRenderer.o build/obj/GLTexture.o build/obj/LevelEditor.o build/obj/Gameplay.o

all: release

release: $(ALLOBJECTS)
	g++ $(RELEASE_FLAGS) $(ALLOBJECTS) -o build/main $(INCLUDES) $(LIBS)

debug: $(ALLOBJECTS)
	g++ $(DEBUG_FLAGS) $(ALLOBJECTS) -o build/main $(INCLUDES) $(LIBS)

build/obj/main.o: main.cpp main.hpp
	g++ -c main.cpp -o build/obj/main.o $(INCLUDES) 

build/obj/bsp.o: bsp.cpp bsp.hpp
	g++ -c bsp.cpp -o build/obj/bsp.o $(INCLUDES) $(LIBS)

build/obj/bsp_collision.o: bsp_collision.cpp bsp_collision.hpp
	g++ -c bsp_collision.cpp -o build/obj/bsp_collision.o $(INCLUDES) $(LIBS)

build/obj/editor_controller.o: editor_controller.cpp editor_controller.hpp
	g++ -c editor_controller.cpp -o build/obj/editor_controller.o $(INCLUDES) $(LIBS)

build/obj/editor_main.o: editor_main.cpp edtitor_main.hpp
	g++ -c editor_main.cpp -o build/obj/editor_main.o $(INCLUDES) $(LIBS)

build/obj/g_main.o: g_main.cpp g_main.hpp
	g++ -c g_main.cpp -o build/obj/g_main.o $(INCLUDES) $(LIBS)

build/obj/game_context.o: game_context.cpp game_context.hpp
	g++ -c game_context.cpp -o build/obj/game_context.o $(INCLUDES) $(LIBS)

build/obj/GAME_main.o: GAME_main.cpp GAME_main.hpp
	g++ -c GAME_main.cpp -o build/obj/GAME_main.o $(INCLUDES) $(LIBS)

build/obj/game_state.o: game_state.cpp game_state.hpp
	g++ -c game_state.cpp -o build/obj/game_state.o $(INCLUDES) $(LIBS)

build/obj/i_input.o: i_input.cpp i_input.hpp
	g++ -c i_input.cpp -o build/obj/i_input.o $(INCLUDES) $(LIBS)

build/obj/light.o: light.cpp light.hpp
	g++ -c light.cpp -o build/obj/light.o $(INCLUDES) $(LIBS)

build/obj/math_utils.o: math_utils.cpp math_utils.hpp
	g++ -c math_utils.cpp -o build/obj/math_utils.o $(INCLUDES)

build/obj/player.o: player.cpp player.hpp
	g++ -c player.cpp -o build/obj/player.o $(INCLUDES) $(LIBS)

build/obj/r_main.o: r_main.cpp r_main.hpp
	g++ -c r_main.cpp -o build/obj/r_main.o $(INCLUDES) $(LIBS)

build/obj/scene_object.o: scene_object.cpp scene_object.hpp
	g++ -c scene_object.cpp -o build/obj/scene_object.o $(INCLUDES) $(LIBS)

build/obj/scene_parser.o: scene_parser.cpp scene_parser.hpp
	g++ -c scene_parser.cpp -o build/obj/scene_parser.o $(INCLUDES) $(LIBS)

build/obj/utils.o: utils.cpp utils.hpp
	g++ -c utils.cpp -o build/obj/utils.o $(INCLUDES) $(LIBS)

build/obj/GLMesh.o: Graphics/GLMesh.cpp Graphics/GLMesh.hpp
	g++ -c Graphics/GLMesh.cpp -o build/obj/GLMesh.o $(INCLUDES) $(LIBS)

build/obj/GLShader.o: Graphics/GLShader.cpp Graphics/GLShader.hpp
	g++ -c Graphics/GLShader.cpp -o build/obj/GLShader.o $(INCLUDES) $(LIBS)

build/obj/GLTextRenderer.o: Graphics/GLTextRenderer.cpp Graphics/GLTextRenderer.hpp
	g++ -c Graphics/GLTextRenderer.cpp -o build/obj/GLTextRenderer.o $(INCLUDES) $(LIBS)

build/obj/GLTexture.o: Graphics/GLTexture.cpp Graphics/GLTexture.hpp
	g++ -c Graphics/GLTexture.cpp -o build/obj/GLTexture.o $(INCLUDES) $(LIBS)

build/obj/Gameplay.o: GameStates/Gameplay.cpp GameStates/Gameplay.hpp
	g++ -c GameStates/Gameplay.cpp -o build/obj/Gameplay.o $(INCLUDES) $(LIBS)

build/obj/LevelEditor.o: GameStates/LevelEditor.cpp GameStates/LevelEditor.hpp
	g++ -c GameStates/LevelEditor.cpp -o build/obj/LevelEditor.o $(INCLUDES) $(LIBS)