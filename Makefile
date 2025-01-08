INCLUDES=-Iinclude -Iinclude/GL -Iinclude/glad -Iinlude/glfw -Iinclude/glm -Iinclude/freetype
LIBS= freetype.dll glad.a libglfw3.a imgui.a -lopengl32  -lgdi32

BASE_FLAGS=
DEBUG_FLAGS=-DEDITOR_DEBUG
ALLOBJECTS=build/obj/main.o build/obj/bsp.o build/obj/bsp_collision.o build/obj/editor_controller.o build/obj/editor_main.o build/obj/g_main.o build/obj/game_context.o build/obj/GAME_main.o build/obj/game_state.o build/obj/i_input.o build/obj/light.o build/obj/math_utils.o build/obj/player.o build/obj/r_main.o build/obj/scene_object.o build/obj/scene_parser.o build/obj/utils.o build/obj/GLMesh.o build/obj/GLShader.o build/obj/GLTextRenderer.o build/obj/GLTexture.o build/obj/LevelEditor.o build/obj/Gameplay.o build/obj/game_audio.o

all: executable
debug: BASE_FLAGS += $(DEBUG_FLAGS)
debug: executable

run: 
	./build/main.exe

executable: $(ALLOBJECTS)
	g++ $(BASE_FLAGS) $(ALLOBJECTS) -o build/main $(INCLUDES) $(LIBS)

build/obj/main.o: main.cpp main.hpp
	g++  $(BASE_FLAGS) -c main.cpp -o build/obj/main.o $(INCLUDES) 

build/obj/bsp.o: bsp.cpp bsp.hpp
	g++ $(BASE_FLAGS) -c bsp.cpp -o build/obj/bsp.o $(INCLUDES)

build/obj/bsp_collision.o: bsp_collision.cpp bsp_collision.hpp
	g++ $(BASE_FLAGS) -c bsp_collision.cpp -o build/obj/bsp_collision.o $(INCLUDES) 

build/obj/editor_controller.o: editor_controller.cpp editor_controller.hpp
	g++ $(BASE_FLAGS) -c editor_controller.cpp -o build/obj/editor_controller.o $(INCLUDES)

build/obj/editor_main.o: editor_main.cpp edtitor_main.hpp
	g++ $(BASE_FLAGS) -c editor_main.cpp -o build/obj/editor_main.o $(INCLUDES)

build/obj/g_main.o: g_main.cpp g_main.hpp
	g++ $(BASE_FLAGS) -c g_main.cpp -o build/obj/g_main.o $(INCLUDES)

build/obj/game_context.o: game_context.cpp game_context.hpp
	g++ $(BASE_FLAGS) -c game_context.cpp -o build/obj/game_context.o $(INCLUDES)

build/obj/GAME_main.o: GAME_main.cpp GAME_main.hpp
	g++ $(BASE_FLAGS) -c GAME_main.cpp -o build/obj/GAME_main.o $(INCLUDES) 

build/obj/game_state.o: game_state.cpp game_state.hpp
	g++ $(BASE_FLAGS) -c game_state.cpp -o build/obj/game_state.o $(INCLUDES)

build/obj/i_input.o: i_input.cpp i_input.hpp
	g++ $(BASE_FLAGS) -c i_input.cpp -o build/obj/i_input.o $(INCLUDES) 

build/obj/light.o: light.cpp light.hpp
	g++ $(BASE_FLAGS) -c light.cpp -o build/obj/light.o $(INCLUDES) 

build/obj/math_utils.o: math_utils.cpp math_utils.hpp
	g++ $(BASE_FLAGS) -c math_utils.cpp -o build/obj/math_utils.o $(INCLUDES)

build/obj/player.o: player.cpp player.hpp
	g++ $(BASE_FLAGS) -c player.cpp -o build/obj/player.o $(INCLUDES)

build/obj/r_main.o: r_main.cpp r_main.hpp
	g++ $(BASE_FLAGS) -c r_main.cpp -o build/obj/r_main.o $(INCLUDES)

build/obj/scene_object.o: scene_object.cpp scene_object.hpp
	g++ $(BASE_FLAGS) -c scene_object.cpp -o build/obj/scene_object.o $(INCLUDES) 

build/obj/scene_parser.o: scene_parser.cpp scene_parser.hpp
	g++ $(BASE_FLAGS) -c scene_parser.cpp -o build/obj/scene_parser.o $(INCLUDES) 

build/obj/utils.o: utils.cpp utils.hpp
	g++ $(BASE_FLAGS) -c utils.cpp -o build/obj/utils.o $(INCLUDES)

build/obj/GLMesh.o: Graphics/GLMesh.cpp Graphics/GLMesh.hpp
	g++ $(BASE_FLAGS) -c Graphics/GLMesh.cpp -o build/obj/GLMesh.o $(INCLUDES) 

build/obj/GLShader.o: Graphics/GLShader.cpp Graphics/GLShader.hpp
	g++ $(BASE_FLAGS) -c Graphics/GLShader.cpp -o build/obj/GLShader.o $(INCLUDES)

build/obj/GLTextRenderer.o: Graphics/GLTextRenderer.cpp Graphics/GLTextRenderer.hpp
	g++ $(BASE_FLAGS) -c Graphics/GLTextRenderer.cpp -o build/obj/GLTextRenderer.o $(INCLUDES) 

build/obj/GLTexture.o: Graphics/GLTexture.cpp Graphics/GLTexture.hpp
	g++ $(BASE_FLAGS) -c Graphics/GLTexture.cpp -o build/obj/GLTexture.o $(INCLUDES) 

build/obj/Gameplay.o: GameStates/Gameplay.cpp GameStates/Gameplay.hpp
	g++ $(BASE_FLAGS) -c GameStates/Gameplay.cpp -o build/obj/Gameplay.o $(INCLUDES)

build/obj/LevelEditor.o: GameStates/LevelEditor/LevelEditor.cpp GameStates/LevelEditor/LevelEditor.hpp
	g++ $(BASE_FLAGS) -c GameStates/LevelEditor/LevelEditor.cpp -o build/obj/LevelEditor.o $(INCLUDES)

build/obj/game_audio.o: game_audio.cpp game_audio.hpp audio_data.hpp
	g++ $(BASE_FLAGS) -c game_audio.cpp -o build/obj/game_audio.o $(INCLUDES)	

clean:
	del build\main.exe
	del /Q /F build\obj\*