#include "MainState.hpp"
#include "../include/glm/glm.hpp"
#include "../math_utils.hpp"
#include "../scene_parser.hpp"

#define WALL_TEST_THRESHOLD 0.11f

scene_data scene;

void Main_Init(game_context & context) 
{ 	
    load_scene("Scenes/test.scene", scene);
	Player_Init(&context);
}

void Main_Update(game_context & context) 
{ 
    if (context.movingMouse && context.rightMouseHeld) 
    {      
        Player_UpdateView(&context);
    }

    Player_UpdateMovementDirection(&context);

    if (sqrMag(main_player.MoveDir) > 0.1f) 
    { 
        glm::vec3 testPos = main_player.Position + WALL_TEST_THRESHOLD * main_player.MoveDir;
        bool canMove = true; 
        float wallSide = 0.0f;

        /* check each wall to make sure the test position doesn't lie behind the wall before moving*/
        for (const room_wall & wall: scene.room_walls)
        {
            wallSide = glm::dot(wall.wall_plane.normal, wall.wall_plane.origin - testPos);
            
            if (wallSide > -WALL_TEST_THRESHOLD) 
            {
                canMove = false;
                break;
            } 
        }

        if (canMove) 
        { 
            Player_UpdatePosition(&context);	
        }
    }
}

void Main_Render(game_context & context) 
{ 
	G_RenderSceneShadowedFull(scene);	
}

void Main_Destroy(game_context & context) 
{ 

}

// main game state, used in the main state machine
game_state gMainState = 
{
    Main_Init,
    Main_Update,
    Main_Render,
    Main_Destroy,
    false, 
    GameState::NONE
};