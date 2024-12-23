#include "MainState.hpp"
#include "../include/glm/glm.hpp"
#include "../math_utils.hpp"

#define WALL_TEST_THRESHOLD 0.11f

void Main_Init(game_context & context) 
{ 	
	Player_Init(&context);
}

#define MAKE_WALL(o,n,w,h) { {n, o}, w, h}
room_wall test_walls[4] = 
{
    MAKE_WALL(glm::vec3(-5,0,0), glm::vec3(1,0,0), 10, 10),
    MAKE_WALL(glm::vec3(5,0,0), glm::vec3(-1,0,0), 10, 10),
    MAKE_WALL(glm::vec3(0,0,5), glm::vec3(0,0,-1), 10, 10),
    MAKE_WALL(glm::vec3(0,0,-5), glm::vec3(0,0,1), 10, 10)
};
#undef MAKE_WALL


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

        /* check each wall to make sure the test position doesn't lie behind the wall before moving*/
        for (int i = 0; i < 4; i++) 
        { 
            float wallSide = glm::dot(test_walls[i].wall_plane.normal, test_walls[i].wall_plane.origin - testPos);
            
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
	G_RenderSceneShadowedFull();	
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