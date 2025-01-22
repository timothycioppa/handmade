#include "LevelEditor.hpp"
#include "editor_render_context.hpp"
#include "../../imgui/imgui.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "editor_utils.hpp"

#define PLACEMENT_STATE(name) { name##_Start, name##_Update, name##_End}

editor_scene_data eScene;
hover_data hoverData;
placement_mode mode;
bool treeBuilt = false;

// used for creating new sectors
sector_e debug = 
{
    glm::vec3(0.0f), glm::vec3(0.0f),
    0.0f, 0.0f,
    glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f)
};

placement_data data;
bool placing = false;

bool checkHoverPosition(glm::vec3 worldPosition, sector_e & box, int sectorIndex, hover_data & hover);
PLACEMENT_STATE_FUNC(FreeMode_Start);
PLACEMENT_STATE_FUNC(FreeMode_Update);
PLACEMENT_STATE_FUNC(FreeMode_End);

PLACEMENT_STATE_FUNC(WallDrag_Start);
PLACEMENT_STATE_FUNC(WallDrag_Update);
PLACEMENT_STATE_FUNC(WallDrag_End);

// stubs, idle does nothing
PLACEMENT_STATE_FUNC(Idle_Start) {}
PLACEMENT_STATE_FUNC(Idle_Update) {}
PLACEMENT_STATE_FUNC(Idle_End) {}

placement_mode_state modeStates[3] = 
{
    PLACEMENT_STATE(FreeMode),
    PLACEMENT_STATE(WallDrag),
    PLACEMENT_STATE(Idle)
};

editor_render_context renderContext;

GAMESTATE_INIT(LevelEditor)
{
	EditorPlayer_Init(&context);
    renderContext.clipToWorld = glm::inverse(gEditorPlayer.camData.projection * gEditorPlayer.camData.view);
    eScene.numSectors = 0;
    eScene.numSegments = 0;
}

int find_sector_recursive(glm::vec3 position, int parentNodeIndex, editor_scene_data & scene) 
{
    bsp_node_e & parentNode = scene.nodes[parentNodeIndex];
    segment_e & parentSegment = scene.segments[parentNode.segment_id];
    glm::vec3 normal = glm::cross(glm::normalize(parentSegment.start - parentSegment.end), {0,1,0}); 
    float testValue = glm::dot(normal, position - parentSegment.start);

    // position is in front of segment
    if (testValue >= 0) 
    {
        if (parentNode.frontID < 0) 
        {
            return parentSegment.frontSectorID;
        }
        else
        {
            return find_sector_recursive(position, parentNode.frontID, scene);
        }
    }
    else
    {
        if (parentNode.backID < 0) 
        {
            return parentSegment.backSectorID;
        }
        else
        {
            return find_sector_recursive(position, parentNode.backID, scene);
        }
    }
}

int find_sector(glm::vec3 position, editor_scene_data & scene) 
{
    return find_sector_recursive(position, 0, scene);
}

int activeSectorID = -1;

GAMESTATE_UPDATE(LevelEditor)
{
    // 1. get world position under cursor
    renderContext.clipToWorld = glm::inverse(gEditorPlayer.camData.projection * gEditorPlayer.camData.view);
    glm::vec3 currentWorldPosition = getWorldCoordinates(gContext.mousePosition, renderContext); 
    data.currentWorldPosition = currentWorldPosition;

 
    if (!placing)
    {

        // reset hover data
        hoverData.type = hover_type::nothing;
        hoverData.sectorIndex = -1;

        for (int i = 0; i < eScene.numSectors; i++) 
        {
            sector_e & sector = eScene.sectors[i];
        
            sector.botColor = glm::vec3(1,1,1);
            sector.leftColor = glm::vec3(1,1,1);
            sector.rightColor = glm::vec3(1,1,1);
            sector.topColor = glm::vec3(1,1,1);
        }

        // figure out what we're hovering over, storing the result in the placement data
        for (int i = 0; i < eScene.numSectors; i++) 
        { 
            sector_e & curr = eScene.sectors[i];

            if (checkHoverPosition(currentWorldPosition, curr, i, hoverData)) 
            {  
                data.sectorIndex  = hoverData.sectorIndex;
                data.sectorSide = hoverData.sectorSide;
                break;
            } 
        }
    }


   if (treeBuilt) 
    {
        activeSectorID = find_sector(currentWorldPosition, eScene);                
    }


    if (mouse_button_held(MouseButtons::M_RIGHT)) 
    {
        placement_mode_state & state = modeStates[int(mode)];
        state.OnUpdate(&data); 
    }


    if (mouse_button_released(MouseButtons::M_RIGHT))
    {
        placing = false;
        placement_mode_state & state = modeStates[int(mode)];
        state.OnEnd(&data);
    }


    if (mouse_button_pressed(MouseButtons::M_RIGHT)) 
    {
        placing = true;
        mode = getCurrentPlacementMode(hoverData.type);
        placement_mode_state & state = modeStates[int(mode)];
        state.OnStart(&data);
    }


    EditorPlayer_UpdateMovementDirection(&context);
    EditorPlayer_UpdatePosition(&context);    
}

GAMESTATE_RENDER(LevelEditor)
{
    renderDebugBox(debug);

    for (int i = 0; i < eScene.numSectors; i++) 
    { 
        sector_e & curr = eScene.sectors[i];
        render_sector(curr, eScene);
    }

    renderContext.cameraPosition = gEditorPlayer.Position;
    renderContext.cursorWorldPosition = data.currentWorldPosition;
    
    G_RenderLevelEditor(renderContext);
}

GAMESTATE_POSTRENDER(LevelEditor){}

GAMESTATE_EDITOR(LevelEditor)
{
    if (ImGui::Button("export", ImVec2(100, 50)))
    {
       exportScene(eScene, "Scenes/test4.scene");
    }

    if (ImGui::Button("build tree", ImVec2(100, 50)))
    {
       build_bsp_tree(eScene);
       treeBuilt = true;
    }

    ImGui::Text("Active sector: %d", activeSectorID);
    ImGui::Text("hovered sector: %d", hoverData.sectorIndex);

    if (hoverData.sectorIndex > -1) 
    {

        sector_e sector = eScene.sectors[hoverData.sectorIndex];
        int segmentID = -1;
        
        switch(hoverData.sectorSide) 
        {
            case sector_side::ss_bot: {segmentID = sector.botSegmentID;} break;
            case sector_side::ss_left: {segmentID = sector.leftSegmentID;} break;
            case sector_side::ss_right: {segmentID = sector.rightSegmentID;} break;
            case sector_side::ss_top: {segmentID = sector.topSegmentID; } break;
        }
        if (segmentID > -1) 
        {

            segment_e segment = eScene.segments[segmentID];

            ImGui::Text("hovered segment: %d", segmentID);
            ImGui::Text("back sector: %d", segment.backSectorID);
            ImGui::Text("front sector: %d", segment.frontSectorID);

        }
    }
}

#define round_to_grid(p) 10.0f * float(glm::floor(p / 10.0f))
#define round_to_grid3(p) {round_to_grid(p.x), round_to_grid(p.y), round_to_grid(p.z)}

GAMESTATE_DESTROY(LevelEditor){}

PLACEMENT_STATE_FUNC(FreeMode_Start)
{ 
    debug.startCorner = pData->currentWorldPosition;
    debug.endCorner = pData->currentWorldPosition;

    debug.startCorner = round_to_grid3(debug.startCorner);
    debug.endCorner = round_to_grid3(debug.endCorner);
}

PLACEMENT_STATE_FUNC(FreeMode_Update)
{ 
    debug.endCorner = pData->currentWorldPosition;
    debug.endCorner = round_to_grid3(debug.endCorner);
}

PLACEMENT_STATE_FUNC(FreeMode_End)
{ 
    // adjust corners to be proper bottom left / top right
    sector_e & newSector = eScene.sectors[eScene.numSectors];
    
    newSector.startCorner = 
    { 
        glm::min(debug.startCorner.x, debug.endCorner.x),
        glm::min(debug.startCorner.y, debug.endCorner.y),
        glm::min(debug.startCorner.z, debug.endCorner.z)
    };

    newSector.endCorner =
    { 
        glm::max(debug.startCorner.x, debug.endCorner.x), 
        glm::max(debug.startCorner.y, debug.endCorner.y), 
        glm::max(debug.startCorner.z, debug.endCorner.z)
    };

    newSector.botColor = glm::vec3(1,0,0);
    newSector.leftColor = glm::vec3(1,0,0);
    newSector.topColor = glm::vec3(1,0,0);
    newSector.rightColor = glm::vec3(1,0,0);
    newSector.width = abs(newSector.startCorner.x - newSector.endCorner.x);
    newSector.height = abs(newSector.startCorner.z - newSector.endCorner.z);

    int sectorIndex = eScene.numSectors;
    eScene.numSectors++;

    segment_e &bot =  eScene.segments[ eScene.numSegments];
    newSector.botSegmentID =  eScene.numSegments;
    bot.frontSectorID = sectorIndex;
    bot.backSectorID = -1;    
    bot.start = newSector.endCorner;
    bot.start.z = newSector.startCorner.z;    
    bot.end = newSector.startCorner;


    segment_e &left =  eScene.segments[ eScene.numSegments+1];
    newSector.leftSegmentID =  eScene.numSegments + 1;
    left.frontSectorID = sectorIndex;
    left.backSectorID = -1;
    left.start = newSector.startCorner;
    left.end = newSector.startCorner;
    left.end.z = newSector.endCorner.z;


    segment_e &top =  eScene.segments[ eScene.numSegments+2];
    newSector.topSegmentID =  eScene.numSegments + 2;
    top.frontSectorID = sectorIndex;
    top.backSectorID = -1;
    top.start = newSector.startCorner;
    top.end = newSector.endCorner;
    top.start.z = newSector.endCorner.z;


    segment_e &right =  eScene.segments[ eScene.numSegments+3];
    newSector.rightSegmentID =  eScene.numSegments + 3;
    right.frontSectorID = sectorIndex;
    right.backSectorID = -1;
    right.start = newSector.endCorner;
    right.end = newSector.endCorner;
    right.end.z = newSector.startCorner.z;

     eScene.numSegments += 4;
}

PLACEMENT_STATE_FUNC(WallDrag_Start)
{ 
    sector_e & sector =  eScene.sectors[pData->sectorIndex];

    switch (pData->sectorSide) 
    {
        case SectorSide::BOTTOM: 
        {
            debug.startCorner = sector.startCorner;
            debug.endCorner = sector.endCorner;
            debug.endCorner.z = sector.startCorner.z;

        } break;
        case SectorSide::RIGHT: 
        {
            debug.startCorner = sector.startCorner;
            debug.endCorner = sector.endCorner;
            debug.startCorner.x = sector.endCorner.x;

         } break;
        case SectorSide::LEFT:
         {
            debug.startCorner = sector.startCorner;
            debug.endCorner = sector.endCorner;
            debug.endCorner.x = sector.startCorner.x;
        } break;
        case SectorSide::TOP: 
        {
            debug.startCorner = sector.startCorner;
            debug.endCorner = sector.endCorner;
            debug.startCorner.z = sector.endCorner.z;
        } break;
    }
}

PLACEMENT_STATE_FUNC(WallDrag_Update)
{ 
    switch (pData->sectorSide) 
    { 
        case SectorSide::TOP: 
        case SectorSide::BOTTOM: 
        {
              debug.endCorner.z = pData->currentWorldPosition.z;
        } break;

        case SectorSide::LEFT: 
        case SectorSide::RIGHT: 
        {
            debug.endCorner.x = pData->currentWorldPosition.x;
        } break;
    }

    debug.endCorner = round_to_grid3(debug.endCorner);
}

PLACEMENT_STATE_FUNC(WallDrag_End)
{ 
    // adjust corners to be proper bottom left / top right
    sector_e & newSector =  eScene.sectors[ eScene.numSectors];
    
    newSector.startCorner = 
    { 
        glm::min(debug.startCorner.x, debug.endCorner.x),
        glm::min(debug.startCorner.y, debug.endCorner.y),
        glm::min(debug.startCorner.z, debug.endCorner.z)
    };

    newSector.endCorner =
    { 
        glm::max(debug.startCorner.x, debug.endCorner.x), 
        glm::max(debug.startCorner.y, debug.endCorner.y), 
        glm::max(debug.startCorner.z, debug.endCorner.z)
    };

    newSector.botColor = glm::vec3(1,0,0);
    newSector.leftColor = glm::vec3(1,0,0);
    newSector.topColor = glm::vec3(1,0,0);
    newSector.rightColor = glm::vec3(1,0,0);
    newSector.width = abs(newSector.startCorner.x - newSector.endCorner.x);
    newSector.height = abs(newSector.startCorner.z - newSector.endCorner.z);

    int sectorIndex =  eScene.numSectors;
    eScene.numSectors++;

    sector_e & connectedSector =  eScene.sectors[hoverData.sectorIndex];

    switch (pData->sectorSide) 
    {
        case sector_side::ss_bot: 
        {

            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &top =  eScene.segments[connectedSector.botSegmentID];
            newSector.topSegmentID = connectedSector.botSegmentID;
            top.backSectorID = sectorIndex; 

            // new segments added for this sector

            segment_e &bot =  eScene.segments[ eScene.numSegments];
            newSector.botSegmentID =  eScene.numSegments;
            bot.frontSectorID = sectorIndex;
            bot.backSectorID = -1;    
            bot.start = newSector.endCorner;
            bot.start.z = newSector.startCorner.z;    
            bot.end = newSector.startCorner;

            segment_e &left =  eScene.segments[ eScene.numSegments+1];
            newSector.leftSegmentID =  eScene.numSegments + 1;
            left.frontSectorID = sectorIndex;
            left.backSectorID = -1;
            left.start = newSector.startCorner;
            left.end = newSector.startCorner;
            left.end.z = newSector.endCorner.z;

            segment_e &right =  eScene.segments[ eScene.numSegments+2];
            newSector.rightSegmentID =  eScene.numSegments + 2;
            right.frontSectorID = sectorIndex;
            right.backSectorID = -1;
            right.start = newSector.endCorner;
            right.end = newSector.endCorner;
            right.end.z = newSector.startCorner.z;

             eScene.numSegments += 3;

            // the rest are new segments
        } break;

        case sector_side::ss_left: 
        {
            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &right =  eScene.segments[connectedSector.leftSegmentID];
            newSector.rightSegmentID = connectedSector.leftSegmentID;
            right.backSectorID = sectorIndex; 

            segment_e &bot =  eScene.segments[ eScene.numSegments];
            newSector.botSegmentID =  eScene.numSegments;
            bot.frontSectorID = sectorIndex;
            bot.backSectorID = -1;    
            bot.start = newSector.endCorner;
            bot.start.z = newSector.startCorner.z;    
            bot.end = newSector.startCorner;


            segment_e &left =  eScene.segments[ eScene.numSegments+1];
            newSector.leftSegmentID =  eScene.numSegments + 1;
            left.frontSectorID = sectorIndex;
            left.backSectorID = -1;
            left.start = newSector.startCorner;
            left.end = newSector.startCorner;
            left.end.z = newSector.endCorner.z;


            segment_e &top =  eScene.segments[ eScene.numSegments+2];
            newSector.topSegmentID =  eScene.numSegments + 2;
            top.frontSectorID = sectorIndex;
            top.backSectorID = -1;
            top.start = newSector.startCorner;
            top.end = newSector.endCorner;
            top.start.z = newSector.endCorner.z;

             eScene.numSegments += 3;


        } break;
        
        case sector_side::ss_top: 
        {
            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &bot =  eScene.segments[connectedSector.topSegmentID];
            newSector.botSegmentID = connectedSector.topSegmentID;
            bot.backSectorID = sectorIndex; 

            segment_e &left =  eScene.segments[ eScene.numSegments];
            newSector.leftSegmentID =  eScene.numSegments;
            left.frontSectorID = sectorIndex;
            left.backSectorID = -1;
            left.start = newSector.startCorner;
            left.end = newSector.startCorner;
            left.end.z = newSector.endCorner.z;


            segment_e &top =  eScene.segments[ eScene.numSegments+1];
            newSector.topSegmentID =  eScene.numSegments + 1;
            top.frontSectorID = sectorIndex;
            top.backSectorID = -1;
            top.start = newSector.startCorner;
            top.end = newSector.endCorner;
            top.start.z = newSector.endCorner.z;


            segment_e &right =  eScene.segments[ eScene.numSegments+2];
            newSector.rightSegmentID =  eScene.numSegments + 2;
            right.frontSectorID = sectorIndex;
            right.backSectorID = -1;
            right.start = newSector.endCorner;
            right.end = newSector.endCorner;
            right.end.z = newSector.startCorner.z;

             eScene.numSegments += 3;


        } break;
        case sector_side::ss_right: 
        {
            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &left =  eScene.segments[connectedSector.rightSegmentID];
            newSector.leftSegmentID = connectedSector.rightSegmentID;
            left.backSectorID = sectorIndex; 

            segment_e &bot =  eScene.segments[ eScene.numSegments];
            newSector.botSegmentID =  eScene.numSegments;
            bot.frontSectorID = sectorIndex;
            bot.backSectorID = -1;    
            bot.start = newSector.endCorner;
            bot.start.z = newSector.startCorner.z;    
            bot.end = newSector.startCorner;

            segment_e &top =  eScene.segments[ eScene.numSegments+1];
            newSector.topSegmentID =  eScene.numSegments + 1;
            top.frontSectorID = sectorIndex;
            top.backSectorID = -1;
            top.start = newSector.startCorner;
            top.end = newSector.endCorner;
            top.start.z = newSector.endCorner.z;


            segment_e &right =  eScene.segments[ eScene.numSegments+2];
            newSector.rightSegmentID =  eScene.numSegments + 2;
            right.frontSectorID = sectorIndex;
            right.backSectorID = -1;
            right.start = newSector.endCorner;
            right.end = newSector.endCorner;
            right.end.z = newSector.startCorner.z;

             eScene.numSegments += 3;

        } break;
    }
}


EXPORT_GAME_STATE(LevelEditor, gStateLevelEditor);
