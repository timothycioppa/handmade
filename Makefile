INCLUDES=-Iinclude -Iinclude/GL -Iinclude/glad -Iinlude/glfw -Iinclude/glm -Iinclude/freetype
LIBS= freetype.dll glad.a libglfw3.a imgui.a -lopengl32  -lgdi32
FLAGS=
ALLOBJECTS=build/obj/main.o build/obj/game_context.o build/obj/light.o build/obj/bsp.o build/obj/editor_main.o build/obj/editor_controller.o build/obj/i_input.o build/obj/scene_object.o build/obj/math_utils.o build/obj/scene_parser.o  build/obj/player.o build/obj/GAME_main.o build/obj/game_state.o build/obj/g_main.o build/obj/r_main.o build/obj/utils.o 

all: game

game: $(ALLOBJECTS)
	g++ $(FLAGS) $(ALLOBJECTS) -o build/main $(INCLUDES) $(LIBS)

build/obj/main.o: main.cpp main.hpp
	g++ -c main.cpp -o build/obj/main.o $(INCLUDES) $(LIBS)

