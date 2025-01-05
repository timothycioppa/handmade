#include "Gameplay.hpp"
#include "../include/glm/glm.hpp"
#include "../math_utils.hpp"
#include "../scene_parser.hpp"
#include "../imgui/imgui.h"
#include "../include/glm/gtc/type_ptr.hpp"
#include "../bsp_collision.hpp"
#define WALL_TEST_THRESHOLD 2.5f
float initialYValue;
float initialYVelocity = 15.0f;

float initialYValueFalling;

bsp_tree scene;

#include "../include/stb_image.h"

void Gameplay_Init(game_context & context) 
{ 	   
    load_scene("Scenes/test3.scene", scene);
    build_bsp_tree(scene);
    ValidateTextures(scene);
	Player_Init(&context);
}

float targetHeight = 0.0f;
float targetHeightFalling = 0.0f;
sector *currentSector = nullptr;
raycast_hit hit;

void OnSectorChanged(sector* oldSector, sector* newSector) 
{
    // newSector shouldn't be allowed to be null. Currently it can be because there's no collision
    // TODO: remove null check after putting collisions in, maybe replace with assertion? 
    // or validate outside and pass a reference rather than a pointer

    if (newSector != nullptr)
    {
        float playerHeight = 2.0f;
        targetHeight = newSector->floorHeight + playerHeight;
        targetHeightFalling = newSector->floorHeight + playerHeight;

        if (!main_player.Jumping) 
        {
            if (oldSector != nullptr)
            {
                if (newSector->floorHeight < oldSector->floorHeight) 
                {
                    main_player.Falling = true;
                    main_player.FallTimer = 0.0f;
                    initialYValueFalling = main_player.Position.y;
                }
            }
        }
    }
}


void Gameplay_Update(game_context & context) 
{ 
    sector *nextSector = get_sector(main_player.Position, scene); 
    
    if (nextSector != currentSector)
    {
        OnSectorChanged(currentSector, nextSector);
        currentSector = nextSector;
    }

    if (bsp_raycast(main_player.Position, main_player.Forward, hit, scene)) 
    {   
        if (hit.hitSegment != nullptr)
        {
            int index0 = hit.hitSegment->renderIndices.renderableIndex0;
            int index1 = hit.hitSegment->renderIndices.renderableIndex1;

            if (hit.RenderType == RenderableType::RT_SOLID_WALL || hit.RenderType == RenderableType::RT_WALL_TOP_SEGMENT)    
            {
                if (index0 > -1) 
                {
                    node_render_data & r = scene.renderables[index0];
                    SET_HIGHLIGHTED(r);
                }    
            }

            if (hit.RenderType == RenderableType::RT_WALL_BOTTOM_SEGMENT )
            {
                if (index1 > -1) 
                {
                    node_render_data & r = scene.renderables[index1];
                    SET_HIGHLIGHTED(r);   
                }    
            }

        }

        if (hit.hitSector != nullptr)
        {
            int index0 = hit.hitSector->renderIndices.renderableIndex0;
            int index1 = hit.hitSector->renderIndices.renderableIndex1;

            if (hit.RenderType == RenderableType::RT_FLOOR )
            {
                if (index0 > -1) 
                {
                    node_render_data & r = scene.renderables[index0];
                    SET_HIGHLIGHTED(r);                
                }    
            }

            if (hit.RenderType == RenderableType::RT_CEILING)
            {
                if (index1 > -1) 
                {
                    node_render_data & r = scene.renderables[index1];
                    SET_HIGHLIGHTED(r);
                }    
            }

        }

        debug_line(glm::vec3(0,0,0), hit.position, glm::vec3(1,0,0), main_player.camData);

    }

    // handle jumping
    if (main_player.Jumping) 
    {   
        float t = main_player.JumpTimer;        
        main_player.Position.y = initialYValue + t * (initialYVelocity - t * 9.8f);      
        main_player.camData.view = glm::lookAt(main_player.Position, main_player.Position + main_player.Forward, {0,1,0});
        main_player.JumpTimer += context.deltaTime;

        if (main_player.Position.y < targetHeight) 
        {             
            main_player.Jumping = false;
            main_player.Position.y = targetHeight;
        }
    }

    // handle falling
    if (main_player.Falling) 
    {   
        float t = main_player.FallTimer;        
        main_player.Position.y = initialYValueFalling - t * t * 9.8f;     
        main_player.camData.view = glm::lookAt(main_player.Position, main_player.Position + main_player.Forward, {0,1,0});
        
        main_player.FallTimer += context.deltaTime;

        if (main_player.Position.y < targetHeightFalling) 
        {             
            main_player.Falling = false;
            main_player.Position.y =  targetHeightFalling;
        }
    }

    // look around while holding the right mouse button
    if (context.movingMouse && mouse_button_held(MouseButtons::M_RIGHT)) 
    {      
        Player_UpdateView(&context);
    }


    // process key input if we're not jumping or falling
    if (!main_player.Jumping && !main_player.Falling) 
    {
        Player_UpdateMovementDirection(&context);
    }


    // collision detection and movement
    if (sqrMag(main_player.MoveDir) > 0.1f) 
    { 
        glm::vec3 testPos = main_player.Position + WALL_TEST_THRESHOLD * main_player.MoveDir;

        if (test_pos_bsp(testPos, scene)) 
         { 
            Player_UpdatePosition(&context);	
        }      
    }


    // jump if not falling or alreading jumping
    if (key_pressed(KeyCodes::KEY_SPACE)) 
    { 
        if (!main_player.Jumping && !main_player.Falling) 
        { 
            main_player.Jumping = true;
            main_player.JumpTimer = 0.0f; 
            initialYValue = main_player.Position.y;        
            targetHeight = initialYValue;
        }  
    }     
}

void clear_single_frame_flags(bsp_tree & tree)
{
    for (int i = 0; i < tree.numRenderables; i++) 
    {
        node_render_data & r = tree.renderables[i];
        r.rendered = false;
        UNSET_HIGHLIGHTED(r);
    }
}

void Gameplay_Render(game_context & context) 
{
   G_RenderSceneShadowedFull(scene);	
}

void Gameplay_PostRender(game_context & context) 
{ 
    clear_single_frame_flags(scene);
}

void playerStats() 
{ 
    ImGui::Begin("Player Stats"); 
 
    glm::vec3 p = main_player.Position;
    glm::vec3 f = main_player.Forward;
    glm::vec3 r = main_player.Right;
    glm::vec3 u = main_player.Up;
    glm::vec3 m = main_player.MoveDir;
 
 
    ImGui::Text("P.Pos:  { %g %g %g }", p.x, p.y, p.z);
    ImGui::Text("P.For:  { %g %g %g }", f.x, f.y, f.z);
    ImGui::Text("P.Rgh:  { %g %g %g }", r.x, r.y, r.z);
    ImGui::Text("P.UP:  { %g %g %g }", u.x, u.y, u.z);
    ImGui::Text("P.MOVE:  { %g %g %g }", m.x, m.y, m.z);
    ImGui::End();
}

    const char* sectortype = "ROOM";
    const char* solidsegmenttype = "WALL";

void Gameplay_Editor(game_context & context) 
{
    #ifdef ASDF

 
   playerStats();

    static char buff[128];

    ImGui::Begin("Tree data"); 


        ImGui::Text("Num nodes: %d", scene.numNodes);

        for (int i = 0; i < scene.numNodes; i++) {ImGui::PushID(i);
            sprintf(buff, "node[%d]", i);
            bsp_node & node = scene.nodes[i];

            if (ImGui::CollapsingHeader(buff))
            {
                NodeType type = FEATURE_TYPE(&node);
                int index = FEATURE_INDEX(&node);  
                ImGui::Text("Type: {%s}", type == NodeType::SECTOR ? sectortype : solidsegmenttype);
                ImGui::Text("FeatureID: {%d}", index);

                switch(type)
                {
                    case NodeType::SECTOR: 
                    {
                        sector & sect = scene.sectors[index];
                        ImGui::Text("rend: %d %d", sect.renderIndices.renderableIndex0, sect.renderIndices.renderableIndex1);
                    } break;
                    case NodeType::WALL_SEGMENT: 
                    {
                        wall_segment & seg = scene.segments[index];                        
                        ImGui::Text("rend: %d %d", seg.renderIndices.renderableIndex0, seg.renderIndices.renderableIndex1);
                    } break;
                }

                if (node.back == nullptr) 
                {
                    ImGui::Text("back: NULL");
                } 
                else
                {
                    ImGui::Text("back: {%d}", node.back->featureIndex.ID);
                }

                if (node.front == nullptr) 
                {
                    ImGui::Text("front: NULL");
                } 
                else
                {
                    ImGui::Text("front: {%d}", node.front->featureIndex.ID);
                }

                
            }
        ImGui::PopID();}    
    
    ImGui::End();

    switch (hit.RenderType) 
    {
        case RenderableType::RT_CEILING: { ImGui::Text("ceiling");} break;
        case RenderableType::RT_FLOOR: { ImGui::Text("floor");} break;
        case RenderableType::RT_SOLID_WALL: { ImGui::Text("solid");} break;
        case RenderableType::RT_WALL_TOP_SEGMENT: { ImGui::Text("top");} break;
        case RenderableType::RT_WALL_BOTTOM_SEGMENT: { ImGui::Text("bot");} break;
    }
   

    // if (hit.featureID > -1)
    // {

    //     wall_segment & segment = scene.segments[hit.featureID];
    //     ImGui::Begin("Raycast Stats");

    //     ImGui::Text("START { %g %g %g }", segment.start.x, segment.start.y, segment.start.z);
    //     ImGui::Text("END { %g %g %g }", segment.end.x, segment.end.y, segment.end.z);
        
    //     ImGui::Text("NORMAL { %g %g %g }", segment.normal.x, segment.normal.y, segment.normal.z);
    //     ImGui::Text("FB { %d %d }", segment.frontSectorID, segment.backSectorID);
        
    //     ImGui::Text("HIT { ID : %d, DIST : %g }", hit.featureID, hit.distance);
    //     ImGui::Text("HITPIS { %g %g %g }", hit.position.x, hit.position.y, hit.position.z);
    //     ImGui::End();

    // }




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

#endif
}

void Gameplay_Destroy(game_context & context) 
{ 
    bsp_tree_free(scene);
}

// main game state, used in the main state machine
game_state gStateGameplay = 
{
    Gameplay_Init,
    Gameplay_Update,
    Gameplay_Render,
    Gameplay_PostRender,
    Gameplay_Editor,
    Gameplay_Destroy,
    false, 
    GameState::NONE
};