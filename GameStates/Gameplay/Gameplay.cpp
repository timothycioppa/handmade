#include "Gameplay.hpp"
#include "../../include/glm/glm.hpp"
#include "../../math_utils.hpp"
#include "../../scene_parser.hpp"
#include "../../imgui/imgui.h"
#include "../../include/glm/gtc/type_ptr.hpp"
#include "../../bsp_collision.hpp"

#define WALL_TEST_THRESHOLD 2.5f

float initialYValue;
float initialYVelocity = 15.0f;
float initialYValueFalling;
bsp_tree scene;
float targetHeight = 0.0f;
float targetHeightFalling = 0.0f;
sector *currentSector = nullptr;
raycast_hit hit;
const char* sectortype = "ROOM";
const char* solidsegmenttype = "WALL";
gameplay_context gameContext;

struct collision_info 
{
    bool hit;
    bool hitEnemy;
    bool hitGeometry;
};


struct player_state
{
    // true whenever the user is TRYING to move 
    bool movementRequested;

    // whether the player's footsteps are currently playing
    bool playingFootsteps;

    // whether we're on the ground or not
    bool grounded;

    // true for the frame when we try to start moving
    bool StartMoving;

    // true for the frame we were moving and we stop
    bool StopMoving;

    // true for the frame we land on the ground, whether from jumping or falling
    bool Landed;

    // true on the frame we start jumping
    bool Jumped;
};

player_state gPlayerState;

// callback when the player moves between sectors
void process_player_state(player_state & state); 
void process_sector_change(sector* oldSector, sector* newSector, player_state & state);
void clear_single_frame_flags(bsp_tree & tree);
void set_hit_highlighted();
void test_collisions(glm::vec3 pos, bsp_tree & tree, collision_info & info) ;
void update_entities() ;
void update_weapon_position() ;

GAMESTATE_INIT(Gameplay)
{ 	   
    load_scene(INITIAL_LEVEL, scene);
    initialize_render_data(scene);

    ValidateTextures(scene);
	Player_Init(&context);
    Audio_LoopSound(SoundCode::BACKGROUND_MUSIC);

    gameContext.primaryWeapon.ableTofire = true;
    gameContext.primaryWeapon.fireDelay = 0.5f;
    gameContext.primaryWeapon.fireTimer = 0.0f;
    gameContext.primaryWeapon.projectileSpeed = 100.0f;

    gameContext.numEntities = 1;

    game_entity & entity = gameContext.Entities[0];
    entity.transform.position = {0, 8, 0};
    entity.boundingBox.center = entity.transform.position;
    entity.boundingBox.extents = {1,1,1};
    entity.MoveSpeed = 1.0f;

    entity.material = 
    {
        get_texture("Textures/wood.jpg"),
        {1,1,1},
        {1,1,1},
        1.0f
    };

    gPlayerState.grounded = true;
    gPlayerState.Jumped = false;
    gPlayerState.Landed = false;
    gPlayerState.movementRequested = false;
    gPlayerState.playingFootsteps = false;
    gPlayerState.StartMoving = false;
    gPlayerState.StopMoving = false;

    gameContext.weaponMaterial = {
        get_texture("Textures/wood.jpg"),
        {1,1,1},
        {1,1,1},
        1.0f
    };
}

GAMESTATE_UPDATE(Gameplay)
{ 
    sector *nextSector = get_sector(main_player.Position, scene); 
    
    if (nextSector != currentSector)
    {
        process_sector_change(currentSector, nextSector, gPlayerState);
        currentSector = nextSector;
    }

    gPlayerState.grounded = !main_player.Jumping && !main_player.Falling;

    // handle jumping
    if (main_player.Jumping) 
    {   
        float t = main_player.JumpTimer;        
        main_player.Position.y = initialYValue + t * (initialYVelocity - t * 9.8f);      
        main_player.camData.view = glm::lookAt(main_player.Position, main_player.Position + main_player.Forward, {0,1,0});
        main_player.JumpTimer += context.deltaTime;

        if (main_player.Position.y < targetHeight) 
        {             
            gPlayerState.Landed = true;
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

    gPlayerState.movementRequested = sqrMag(main_player.MoveDir) > 0.1f;

    // collision detection and movement
    if (gPlayerState.movementRequested) 
    { 
        if (!main_player.Moving) 
        {
            gPlayerState.StartMoving = true;
            main_player.Moving = true;            
        }

        glm::vec3 testPos = main_player.Position + WALL_TEST_THRESHOLD * main_player.MoveDir;

        if (test_pos_bsp(testPos, scene)) 
        { 
            Player_UpdatePosition(&context);	
        }      
    } 
    else  
    { 
        if (main_player.Moving) 
        {
            gPlayerState.StopMoving = true;
        }
        main_player.Moving = false;
    }

    // jump if not falling or alreading jumping
    if (key_pressed(KeyCodes::KEY_SPACE)) 
    { 
        if (gPlayerState.grounded)
        { 
            gPlayerState.Jumped = true;
            main_player.Jumping = true;
            main_player.JumpTimer = 0.0f; 
            initialYValue = main_player.Position.y;        
            targetHeight = initialYValue;
          
        }  
    }     

    weapon & primaryWeapon = gameContext.primaryWeapon;

    if (gameContext.explosion.alive) 
    {
        particle_simulation_context p_context = 
        {
            gContext.deltaTime,
            main_player.Position
        };

        simulate_system(gameContext.explosion, p_context); 
    }

    collision_info collisionInfo = {false, false, false};

    // update all active projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) 
    {
        projectile & p = primaryWeapon.projectiles[i];

        if (p.active) 
        { 
            float speed = primaryWeapon.projectileSpeed;
            p.position += (p.direction * speed * gContext.deltaTime);
            p.age += gContext.deltaTime;

            test_collisions(p.position, scene, collisionInfo);

            if (collisionInfo.hit) 
            { 
                p.age = 0.0f;
                p.active = false;

                system_spawn_info info;
                {
                    info.initialPosition = p.position;
                    info.ageRange   = { 0.25f,  0.5f  };
                    info.speedRange = { 15.0f,  25.0f };
                                       
                    init_system(gameContext.explosion, info);
                }

                Audio_PlaySound(SoundCode::PROJECTILE_EXPLOSION);

                if (collisionInfo.hitEnemy)
                {
                    Audio_PlaySound(SoundCode::HIT_ENEMY);
                }
            }

            // lived too long, despawn projectile 
            else if (p.age > p.lifetime) 
            { 
                p.age = 0.0f;
                p.active = false;
            }
        }
    }

    // process weapon fire delay, determine if we are able to fire again.

    if (primaryWeapon.fireTimer > 0.0f) 
    {
        primaryWeapon.fireTimer -= gContext.deltaTime;
        
        if (primaryWeapon.fireTimer < 0.0f) 
        { 
            primaryWeapon.fireTimer = 0.0f;
            primaryWeapon.ableTofire = true;
        }
    }

    // try to fire a projectile
    if (mouse_button_pressed(MouseButtons::M_LEFT)) 
    {
        // fire weapon
        if (primaryWeapon.ableTofire) 
        {        
            projectile* p = nullptr;

            // find first free projectile
            for (int i = 0; i < MAX_PROJECTILES; i++) 
            {
                projectile & proj = primaryWeapon.projectiles[i];
                if (!proj.active) 
                {
                    p = &proj;
                    break;
                }
            }

            // fonud a projectile
            if (p != nullptr) 
            { 
                p->active = true;
                p->position = gameContext.weaponTransform.position;
                p->direction = gameContext.weaponAimDir;
                p->age = 0.0f;
                p->lifetime = 5.0f;

                primaryWeapon.fireTimer = primaryWeapon.fireDelay;
                primaryWeapon.ableTofire = false;
            }    
        }                  
    }

    update_entities();
    update_weapon_position();
    process_player_state(gPlayerState);
}

GAMESTATE_RENDER(Gameplay)
{    
    G_RenderSceneShadowedFull(scene, gameContext);	
}

GAMESTATE_POSTRENDER(Gameplay)
{ 
    clear_single_frame_flags(scene);
}

GAMESTATE_EDITOR(Gameplay)
{

    static char buff[128];

  // playerStats();


    // ImGui::Begin("Tree data"); 


    //     ImGui::Text("Num nodes: %d", scene.numNodes);

    //     for (int i = 0; i < scene.numNodes; i++) {ImGui::PushID(i);
    //         sprintf(buff, "node[%d]", i);
    //         bsp_node & node = scene.nodes[i];

    //         if (ImGui::CollapsingHeader(buff))
    //         {
    //             NodeType type = FEATURE_TYPE(&node);
    //             int index = FEATURE_INDEX(&node);  
    //             ImGui::Text("Type: {%s}", type == NodeType::SECTOR ? sectortype : solidsegmenttype);
    //             ImGui::Text("FeatureID: {%d}", index);

    //             switch(type)
    //             {
    //                 case NodeType::SECTOR: 
    //                 {
    //                     sector & sect = scene.sectors[index];
    //                     ImGui::Text("rend: %d %d", sect.renderIndices.renderableIndex0, sect.renderIndices.renderableIndex1);
    //                 } break;
    //                 case NodeType::WALL_SEGMENT: 
    //                 {
    //                     wall_segment & seg = scene.segments[index];                        
    //                     ImGui::Text("rend: %d %d", seg.renderIndices.renderableIndex0, seg.renderIndices.renderableIndex1);
    //                 } break;
    //             }

    //             if (node.back == nullptr) 
    //             {
    //                 ImGui::Text("back: NULL");
    //             } 
    //             else
    //             {
    //                 ImGui::Text("back: {%d}", node.back->featureIndex.ID);
    //             }

    //             if (node.front == nullptr) 
    //             {
    //                 ImGui::Text("front: NULL");
    //             } 
    //             else
    //             {
    //                 ImGui::Text("front: {%d}", node.front->featureIndex.ID);
    //             }

                
    //         }
    //     ImGui::PopID();}    
    
    // ImGui::End();

    // switch (hit.RenderType) 
    // {
    //     case RenderableType::RT_CEILING: { ImGui::Text("ceiling");} break;
    //     case RenderableType::RT_FLOOR: { ImGui::Text("floor");} break;
    //     case RenderableType::RT_SOLID_WALL: { ImGui::Text("solid");} break;
    //     case RenderableType::RT_WALL_TOP_SEGMENT: { ImGui::Text("top");} break;
    //     case RenderableType::RT_WALL_BOTTOM_SEGMENT: { ImGui::Text("bot");} break;
    // }
   
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

    // ImGui::Begin("player");    
    //     glm::vec3 p = main_player.Position;
    //     glm::vec3 f = main_player.Forward;
    //     glm::vec3 r = main_player.Right;
    //     glm::vec3 u = main_player.Up;
    //     glm::vec3 m = main_player.MoveDir;


    //     ImGui::Text("Position: %g %g %g", p.x, p.y, p.z );
    //     ImGui::Text("Forward: %g %g %g", f.x, f.y, f.z );
    //     ImGui::Text("Right: %g %g %g", r.x, r.y, r.z );
    //     ImGui::Text("Up: %g %g %g", u.x, u.y, u.z );
    //     ImGui::Text("Movedir: %g %g %g", m.x, m.y, m.z );


    // ImGui::End();
}

GAMESTATE_DESTROY(Gameplay)
{ 
    bsp_tree_free(scene);
}

EXPORT_GAME_STATE(Gameplay, gStateGameplay);

void process_sector_change(sector* oldSector, sector* newSector, player_state & state) 
{
    if (newSector != nullptr)
    {
        float playerHeight = 2.0f;
        targetHeight = newSector->floorHeight + playerHeight;
        targetHeightFalling = newSector->floorHeight + playerHeight;

        if (!main_player.Jumping) 
        {
            if (oldSector != nullptr)
            {
                // start falling 
                if (newSector->floorHeight < oldSector->floorHeight) 
                {
                    state.Landed = true;
                    main_player.Falling = true;
                    main_player.FallTimer = 0.0f;
                    initialYValueFalling = main_player.Position.y;
                }

                // step up to a new higher sector
                if (!main_player.Falling) 
                {
                    if (newSector->floorHeight > oldSector->floorHeight)
                    {
                        main_player.Position.y = newSector->floorHeight + playerHeight;
                    }
                }
            }
        }
    }
}

void set_hit_highlighted()
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

void test_collisions(glm::vec3 pos, bsp_tree & tree, collision_info & info) 
{
    info.hit = !test_pos_bsp(pos, tree);

    if (!info.hit) 
    {
        for (int i = 0; i < gameContext.numEntities; i++) 
        {
            if (aabb_contains(pos, gameContext.Entities[i].boundingBox))
            {
                Audio_PlaySound(SoundCode::HIT_ENEMY);
                info.hit = true;
                info.hitEnemy = true;
                break;
            }
        }
    } else
    {
        info.hitGeometry = true;
    }
}

void update_entities() 
{ 
    for (int i = 0; i < gameContext.numEntities; i++) 
    {
        game_entity & entity = gameContext.Entities[i];
        glm::vec3 & enemyPos = entity.transform.position;
        glm::vec3 lookDir = glm::normalize(main_player.Position -  enemyPos);
       
        enemyPos += lookDir * gContext.deltaTime * entity.MoveSpeed;;
        entity.boundingBox.center = enemyPos;
        gameContext.Entities[i].transform.localToWorld = glm::inverse(glm::lookAt(enemyPos, enemyPos + lookDir, {0,1,0}));       
    }
}

void process_player_state(player_state & state) 
{ 
    if (state.Jumped) 
    {
        Audio_PlaySound(SoundCode::PLAYER_JUMP);
    }

    if (state.playingFootsteps) 
    {
        if (!state.grounded)
        {
            state.playingFootsteps = false;
            Audio_StopSound(SoundCode::PLAYER_RUN);
        }

        if (state.StopMoving) 
        {
            state.playingFootsteps = false;
            Audio_StopSound(SoundCode::PLAYER_RUN);
        }
    }
    else 
    {
        if (state.Landed) 
        {
            if (state.movementRequested) 
            {
                 state.playingFootsteps = true;
                 Audio_LoopSound(SoundCode::PLAYER_RUN);
            }
        }

        if (state.StartMoving) 
        {
            if (state.grounded) 
            {
                state.playingFootsteps = true;
                Audio_LoopSound(SoundCode::PLAYER_RUN);
            }
        }
    }

    // transient flags, these should be reset after processing since they can only happen once
    gPlayerState.StartMoving = false;
    gPlayerState.StopMoving = false;
    gPlayerState.Landed = false;
    gPlayerState.Jumped = false;
}

float amp = 0.05f;
float speed = 8.0f;
glm::vec3 weaponScale = {.05, .1, .8};
glm::vec3 weaponOffset = {0.5f, 0.0f, 0.25f};

void update_weapon_position() 
{ 
    float bobOffset = sin(speed * gContext.applicationTime) * amp;   
    
    glm::vec3 weaponPos = 
        main_player.Position + 
            main_player.Right * weaponOffset.x + 
            glm::vec3(0.0f, bobOffset + weaponOffset.y , 0.0f)  + 
            main_player.Forward * weaponOffset.z;
    
    glm::vec3 target = main_player.Position + 20.0f * main_player.Forward;

    gameContext.weaponTransform.position = weaponPos;
    gameContext.weaponAimDir = glm::normalize(target - weaponPos);
    gameContext.weaponTransform.localToWorld = glm::scale(glm::mat4(1.0f), weaponScale);
    gameContext.weaponTransform.localToWorld =  glm::inverse(glm::lookAt(weaponPos, weaponPos + gameContext.weaponAimDir, {0,1,0})) *  gameContext.weaponTransform.localToWorld;

}
