#include "Gameplay.hpp"
#include "../include/glm/glm.hpp"
#include "../math_utils.hpp"
#include "../scene_parser.hpp"
#include "../imgui/imgui.h"
#include "../include/glm/gtc/type_ptr.hpp"
#include "../bsp.hpp"

#define WALL_TEST_THRESHOLD 2.5f
scene_data scene;
float initialYValue;
float initialYVelocity = 15.0f;
bsp_tree bsp;

void build_bsp_tree(bsp_tree & tree);

void Gameplay_Init(game_context & context) 
{ 	
    load_scene("Scenes/test.scene", scene);
	Player_Init(&context);
    build_bsp_tree(bsp);
}

void Gameplay_Update(game_context & context) 
{ 
    raycast_hit hit;

    if (bsp_raycast(main_player.Position, main_player.Forward, hit, bsp)) 
    {
        debug_line(glm::vec3(0,0,0), hit.position, glm::vec3(1,0,0), main_player.camData);               
    }

    if (main_player.Jumping) 
    {         
        float t = main_player.JumpTimer;        
        main_player.Position.y = initialYValue + t * (initialYVelocity - t * 9.8f);      
        main_player.camData.view = glm::lookAt(main_player.Position, main_player.Position + main_player.Forward, main_player.Up);
        main_player.JumpTimer += context.deltaTime;

        if (main_player.Position.y < initialYValue) 
        {             
            main_player.Jumping = false;
            main_player.Position.y = initialYValue;
        }
    }

    if (context.movingMouse && mouse_button_held(MouseButtons::M_RIGHT)) 
    {      
        Player_UpdateView(&context);
    }

    if (!main_player.Jumping) 
    {
        Player_UpdateMovementDirection(&context);
    }

    if (sqrMag(main_player.MoveDir) > 0.1f) 
    { 
        glm::vec3 testPos = main_player.Position + WALL_TEST_THRESHOLD * main_player.MoveDir;

        if (test_pos_bsp(testPos, bsp)) 
        { 
            Player_UpdatePosition(&context);	
        }      
    }

    if (key_pressed(KeyCodes::KEY_SPACE)) 
    { 
        if (!main_player.Jumping) 
        { 
            main_player.Jumping = true;
            main_player.JumpTimer = 0.0f; 
            initialYValue = main_player.Position.y;        
        }  
    }     
}

void Gameplay_Render(game_context & context) 
{ 
	G_RenderSceneShadowedFull(scene);	
}

void Gameplay_Editor(game_context & context) 
{
    static char buff[128];
    ImGui::Begin("lights");
    
    for (int i = 0; i < 3; i++) {
        
        ImGui::PushID(i);
        sprintf(buff, "light[%d]", i);
        
        if (ImGui::CollapsingHeader(buff)) 
        { 
            glm::vec3 & lightPos = scene.lights[i].Position;
            glm::vec3 & lightColor = scene.lights[i].Color;
            float & intensity = scene.lights[i].intensity;
            
            if (ImGui::DragFloat3("position", glm::value_ptr(lightPos), 0.01f, -20.0f, 20.0f)) 
            { 
                update_light_direction(scene.lights[0]);
                update_projection_matrix(scene.lights[0]);
            }  

            ImGui::DragFloat3("color", glm::value_ptr(lightColor), 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("intensity", &intensity, 0.01f, 0.0f, 100.0f);
        }

    
        ImGui::PopID();
    }
    ImGui::End();

}

void Gameplay_Destroy(game_context & context) 
{ 
    bsp_tree_free(bsp);
}

void build_bsp_tree(bsp_tree & tree) 
{ 
    tree.numSegments = 4;
    tree.segments = (wall_segment *) malloc (tree.numSegments * sizeof(wall_segment));
   
    tree.segments[0] =  {{-20,0, -20}, {-20,0, 20}, {1, 0, 0}};
    tree.segments[1] = {{-20,0, 20}, {20,0, 20}, {0, 0, -1}};
    tree.segments[2] = {{20, 0, 20}, {20, 0,-20}, {-1,0,  0}};
    tree.segments[3] = {{20, 0, -20}, {-20,0, -20}, {0,0, 1}};

    tree.numNodes = 9;
    tree.nodes = (bsp_node *) malloc (tree.numNodes * sizeof(bsp_node));
    bsp_node * _nodes = tree.nodes;  

    tree.numSectors = 1;
    tree.sectors = (sector*) malloc(tree.numSectors * sizeof(sector));
    tree.sectors[0] = {0.0f, 20.0f};
    int sector_id = 0;

    bsp_node * s0 = &_nodes[0]; 
    bsp_node * s1 = &_nodes[1];
    bsp_node * s2 = &_nodes[2];
    bsp_node * s3 = &_nodes[3];
    bsp_node * s4 = &_nodes[4];
    bsp_node * s5 = &_nodes[5];
    bsp_node * s6 = &_nodes[6];
    bsp_node * s7 = &_nodes[7];
    bsp_node * s8 = &_nodes[8];

    SPLIT_NODE(s0, 0, s1, s2)
    FILLED_NODE(s1)
    SPLIT_NODE(s2, 1, s3, s4)
    FILLED_NODE(s3)
    SPLIT_NODE(s4, 2, s5, s6)
    FILLED_NODE(s5)
    SPLIT_NODE(s6, 3, s7, s8)
    FILLED_NODE(s7)
    SECTOR_NODE(s8, 0)
    
    tree.root = s0;
}


// main game state, used in the main state machine
game_state gStateGameplay = 
{
    Gameplay_Init,
    Gameplay_Update,
    Gameplay_Render,
    Gameplay_Editor,
    Gameplay_Destroy,
    false, 
    GameState::NONE
};