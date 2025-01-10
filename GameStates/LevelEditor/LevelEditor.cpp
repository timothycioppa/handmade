#include "LevelEditor.hpp"
#include "editor_render_context.hpp"
#include "../../imgui/imgui.h"
#include <fstream>
#include <iostream>
#include "editor_utils.hpp"

#define PLACEMENT_STATE(name) { name##_Start, name##_Update, name##_End}

editor_scene_data eScene;
hover_data hoverData;
placement_mode mode;

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
    if (ImGui::Button("export", ImVec2(100, 50))){
       exportScene(eScene, "Scenes/test4.scene");
    }
}

GAMESTATE_DESTROY(LevelEditor){}

PLACEMENT_STATE_FUNC(FreeMode_Start)
{ 
    debug.startCorner = pData->currentWorldPosition;
    debug.endCorner = pData->currentWorldPosition;
}

PLACEMENT_STATE_FUNC(FreeMode_Update)
{ 
    debug.endCorner = pData->currentWorldPosition;
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
    newSector.width = glm::abs(newSector.startCorner.x - newSector.endCorner.x);
    newSector.height = glm::abs(newSector.startCorner.z - newSector.endCorner.z);

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
    newSector.width = glm::abs(newSector.startCorner.x - newSector.endCorner.x);
    newSector.height = glm::abs(newSector.startCorner.z - newSector.endCorner.z);

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
