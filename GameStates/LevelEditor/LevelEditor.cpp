#include "LevelEditor.hpp"
#include "editor_render_context.hpp"
#include "../../imgui/imgui.h"
#include <fstream>
#include <iostream>

sector_e sectors[MAX_EDITOR_SECTORS];
segment_e segments[MAX_EDITOR_SEGMENTS];

int numSectors;
int numSegments;

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

glm::vec3 getWorldCoordinates(glm::vec2 & windowCoordinates);
bool checkHoverPosition(glm::vec3 worldPosition, sector_e & box, int sectorIndex, hover_data & hover);
void render_box(sector_e & box);
placement_mode getCurrentPlacementMode(hover_type type);

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



void exportScene() 
{ 
    std::ofstream stream("Scenes/test4.scene");
    stream << "sectors" << std::endl;
    stream << "count " << numSectors << std::endl;

    for (int i = 0; i < numSectors; i++) 
    {
        sector_e & s = sectors[i];
        float cx = s.startCorner.x + 0.5f * (s.endCorner.x - s.startCorner.x);
        float cy = s.startCorner.z + 0.5f * (s.endCorner.z - s.startCorner.z);
        float floor = 0.0f;
        float ceiling = 20.0f;

        stream  
            << "floor " << floor 
            << " ceiling " << ceiling 
            << " center " << cx << " " << cy << " "
            << " dimension " << s.width << " " << s.height << " "
            << " wall " << s.botSegmentID << " " << s.leftSegmentID << " " << s.topSegmentID << " " << s.rightSegmentID << std::endl;
    }

    stream << "segments" << std::endl;
    stream << "count " << numSegments << std::endl;

    for (int i = 0; i < numSegments; i++) 
    {
        segment_e & s = segments[i];

        stream
            << "s " << s.start.x << " " << s.start.z  << " "
            << "e " << s.end.x << " " << s.end.z << " "
            << "f " << s.frontSectorID << " " << "b " << s.backSectorID << std::endl;    
    }

    stream.close();
}

#define PLACEMENT_STATE(name) { name##_Start, name##_Update, name##_End}

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
    numSectors = 0;
    numSegments = 0;
}

GAMESTATE_UPDATE(LevelEditor)
{
    // 1. get world position under cursor
    renderContext.clipToWorld = glm::inverse(gEditorPlayer.camData.projection * gEditorPlayer.camData.view);
    glm::vec3 currentWorldPosition = getWorldCoordinates(gContext.mousePosition); 
    data.currentWorldPosition = currentWorldPosition;

    if (!placing)
    {

        // reset hover data
        hoverData.type = hover_type::nothing;
        hoverData.sectorIndex = -1;

        for (int i = 0; i < numSectors; i++) 
        {
            sector_e & sector = sectors[i];
        
            sector.botColor = glm::vec3(1,1,1);
            sector.leftColor = glm::vec3(1,1,1);
            sector.rightColor = glm::vec3(1,1,1);
            sector.topColor = glm::vec3(1,1,1);
        }


        // figure out what we're hovering over, storing the result in the placement data
        for (int i = 0; i < numSectors; i++) 
        { 
            sector_e & curr = sectors[i];

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
    render_box(debug);

    for (int i = 0; i < numSectors; i++) 
    { 
        sector_e & curr = sectors[i];
        render_box(curr);
    }

    renderContext.cameraPosition = gEditorPlayer.Position;
    G_RenderLevelEditor(renderContext);
}

GAMESTATE_POSTRENDER(LevelEditor){}
GAMESTATE_EDITOR(LevelEditor)
{
    if (ImGui::Button("export", ImVec2(100, 50))){
       exportScene();
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
    sector_e & newSector = sectors[numSectors];
    
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

    int sectorIndex = numSectors;
    numSectors++;

    segment_e &bot = segments[numSegments];
    newSector.botSegmentID = numSegments;
    bot.frontSectorID = sectorIndex;
    bot.backSectorID = -1;    
    bot.start = newSector.endCorner;
    bot.start.z = newSector.startCorner.z;    
    bot.end = newSector.startCorner;


    segment_e &left = segments[numSegments+1];
    newSector.leftSegmentID = numSegments + 1;
    left.frontSectorID = sectorIndex;
    left.backSectorID = -1;
    left.start = newSector.startCorner;
    left.end = newSector.startCorner;
    left.end.z = newSector.endCorner.z;


    segment_e &top = segments[numSegments+2];
    newSector.topSegmentID = numSegments + 2;
    top.frontSectorID = sectorIndex;
    top.backSectorID = -1;
    top.start = newSector.startCorner;
    top.end = newSector.endCorner;
    top.start.z = newSector.endCorner.z;


    segment_e &right = segments[numSegments+3];
    newSector.rightSegmentID = numSegments + 3;
    right.frontSectorID = sectorIndex;
    right.backSectorID = -1;
    right.start = newSector.endCorner;
    right.end = newSector.endCorner;
    right.end.z = newSector.startCorner.z;

    numSegments += 4;
}

PLACEMENT_STATE_FUNC(WallDrag_Start)
{ 
    sector_e & sector = sectors[pData->sectorIndex];

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
    sector_e & newSector = sectors[numSectors];
    
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

    int sectorIndex = numSectors;
    numSectors++;

    sector_e & connectedSector = sectors[hoverData.sectorIndex];

    switch (pData->sectorSide) 
    {
        case sector_side::ss_bot: 
        {
            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &top = segments[connectedSector.botSegmentID];
            newSector.topSegmentID = connectedSector.botSegmentID;
            top.backSectorID = sectorIndex; 

            // new segments added for this sector

            segment_e &bot = segments[numSegments];
            newSector.botSegmentID = numSegments;
            bot.frontSectorID = sectorIndex;
            bot.backSectorID = -1;    
            bot.start = newSector.endCorner;
            bot.start.z = newSector.startCorner.z;    
            bot.end = newSector.startCorner;

            segment_e &left = segments[numSegments+1];
            newSector.leftSegmentID = numSegments + 1;
            left.frontSectorID = sectorIndex;
            left.backSectorID = -1;
            left.start = newSector.startCorner;
            left.end = newSector.startCorner;
            left.end.z = newSector.endCorner.z;

            segment_e &right = segments[numSegments+2];
            newSector.rightSegmentID = numSegments + 2;
            right.frontSectorID = sectorIndex;
            right.backSectorID = -1;
            right.start = newSector.endCorner;
            right.end = newSector.endCorner;
            right.end.z = newSector.startCorner.z;

            numSegments += 3;

            // the rest are new segments
        } break;

        case sector_side::ss_left: 
        {
            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &right = segments[connectedSector.leftSegmentID];
            newSector.rightSegmentID = connectedSector.leftSegmentID;
            right.backSectorID = sectorIndex; 

            segment_e &bot = segments[numSegments];
            newSector.botSegmentID = numSegments;
            bot.frontSectorID = sectorIndex;
            bot.backSectorID = -1;    
            bot.start = newSector.endCorner;
            bot.start.z = newSector.startCorner.z;    
            bot.end = newSector.startCorner;


            segment_e &left = segments[numSegments+1];
            newSector.leftSegmentID = numSegments + 1;
            left.frontSectorID = sectorIndex;
            left.backSectorID = -1;
            left.start = newSector.startCorner;
            left.end = newSector.startCorner;
            left.end.z = newSector.endCorner.z;


            segment_e &top = segments[numSegments+2];
            newSector.topSegmentID = numSegments + 2;
            top.frontSectorID = sectorIndex;
            top.backSectorID = -1;
            top.start = newSector.startCorner;
            top.end = newSector.endCorner;
            top.start.z = newSector.endCorner.z;

            numSegments += 3;


        } break;
        
        case sector_side::ss_top: {
            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &bot = segments[connectedSector.topSegmentID];
            newSector.botSegmentID = connectedSector.topSegmentID;
            bot.backSectorID = sectorIndex; 




            segment_e &left = segments[numSegments];
            newSector.leftSegmentID = numSegments;
            left.frontSectorID = sectorIndex;
            left.backSectorID = -1;
            left.start = newSector.startCorner;
            left.end = newSector.startCorner;
            left.end.z = newSector.endCorner.z;


            segment_e &top = segments[numSegments+1];
            newSector.topSegmentID = numSegments + 1;
            top.frontSectorID = sectorIndex;
            top.backSectorID = -1;
            top.start = newSector.startCorner;
            top.end = newSector.endCorner;
            top.start.z = newSector.endCorner.z;


            segment_e &right = segments[numSegments+2];
            newSector.rightSegmentID = numSegments + 2;
            right.frontSectorID = sectorIndex;
            right.backSectorID = -1;
            right.start = newSector.endCorner;
            right.end = newSector.endCorner;
            right.end.z = newSector.startCorner.z;

            numSegments += 3;


        } break;
        case sector_side::ss_right: 
        {
            // SHARED SEGMENT: this should be the bottom of the segment we dragged off of
            segment_e &left = segments[connectedSector.rightSegmentID];
            newSector.leftSegmentID = connectedSector.rightSegmentID;
            left.backSectorID = sectorIndex; 

            segment_e &bot = segments[numSegments];
            newSector.botSegmentID = numSegments;
            bot.frontSectorID = sectorIndex;
            bot.backSectorID = -1;    
            bot.start = newSector.endCorner;
            bot.start.z = newSector.startCorner.z;    
            bot.end = newSector.startCorner;

            segment_e &top = segments[numSegments+1];
            newSector.topSegmentID = numSegments + 1;
            top.frontSectorID = sectorIndex;
            top.backSectorID = -1;
            top.start = newSector.startCorner;
            top.end = newSector.endCorner;
            top.start.z = newSector.endCorner.z;


            segment_e &right = segments[numSegments+2];
            newSector.rightSegmentID = numSegments + 2;
            right.frontSectorID = sectorIndex;
            right.backSectorID = -1;
            right.start = newSector.endCorner;
            right.end = newSector.endCorner;
            right.end.z = newSector.startCorner.z;

            numSegments += 3;

        } break;
    }
}

glm::vec3 getWorldCoordinates(glm::vec2 & windowCoordinates) 
{ 
    // convert window coordinates to normalized device coords in [-1, 1]
    glm::vec3 ndc;
    ndc.x = ((2.0f * windowCoordinates.x) / gContext.windowWidth - 1.0f);
    ndc.y = 1.0f - (2.0f * windowCoordinates.y) / gContext.windowHeight;
    ndc.z = -1.0f; 
    glm::vec4 worldPos = renderContext.clipToWorld * glm::vec4(ndc, 1.0f);
    return  {worldPos.x, worldPos.y, worldPos.z};
}

bool checkHoverPosition(glm::vec3 worldPosition, sector_e & box, int sectorIndex, hover_data & hover)
{
    glm::vec3 & start = box.startCorner;
    glm::vec3 & end = box.endCorner;
    float thresh = 0.35f;
    hover.type = hover_type::nothing;

    // check bottom and top lines
    if (worldPosition.x >= start.x && worldPosition.x <= end.x) 
    { 
        if (glm::abs(worldPosition.z - start.z) < thresh)
        {
            box.botColor = glm::vec3(1,0,0);        
            hover.type = hover_type::wall_segment;
            hover.sectorIndex = sectorIndex;
            hover.sectorSide = sector_side::ss_bot;
            
            return true;
        }
        else if (glm::abs(worldPosition.z - end.z) < thresh)
        {
            box.topColor = glm::vec3(1,1,0);      
            hover.type = hover_type::wall_segment;
            hover.sectorIndex = sectorIndex;
            hover.sectorSide = sector_side::ss_top;

            return true;
        }
        else
         { 
            box.botColor = glm::vec3(1,1,1);        
            box.topColor = glm::vec3(1,1,1);        
        } 
        
    } else 
    { 
          box.botColor = glm::vec3(1,1,1);    
          box.topColor = glm::vec3(1,1,1);        
    }

     // check bottom and top lines
    if (worldPosition.z >= start.z && worldPosition.z <= end.z) 
    {
        if (glm::abs(worldPosition.x - start.x) < thresh)
        {
            box.leftColor = glm::vec3(1,0,0);     
            hover.type = hover_type::wall_segment;
            hover.sectorIndex = sectorIndex;
            hover.sectorSide = sector_side::ss_left;


            return true;
        }
        else if (glm::abs(worldPosition.x - end.x) < thresh)
        {
            box.rightColor = glm::vec3(1,1,0);        
            hover.type = hover_type::wall_segment;
            hover.sectorIndex = sectorIndex;            
            hover.sectorSide = sector_side::ss_right;

            return true;
        }
        else
        { 
            box.leftColor = glm::vec3(1,1,1);        
            box.rightColor = glm::vec3(1,1,1);        
        }  
    } else { 
          box.leftColor = glm::vec3(1,1,1);    
          box.rightColor = glm::vec3(1,1,1);        
    }

    return false;
}

void render_box(sector_e & box) 
{
    glm::vec3 &s = box.startCorner;
    glm::vec3 &e = box.endCorner;

    #define BL  glm::vec3(s.x, 0.0f, s.z)
    #define BR  glm::vec3(e.x, 0.0f, s.z)
    #define TL  glm::vec3(s.x, 0.0f, e.z)
    #define TR  glm::vec3(e.x, 0.0f, e.z)

    debug_line(BL, BR,  box.botColor,   gEditorPlayer.camData);
    debug_line(TL, TR,  box.topColor,   gEditorPlayer.camData);
    debug_line(BL, TL,  box.leftColor,  gEditorPlayer.camData);        
    debug_line(BR, TR,  box.rightColor, gEditorPlayer.camData);
}

placement_mode getCurrentPlacementMode(hover_type type)
{
    switch (type)
    {

        // hovering over nothing, just start freely making a sector
        case hover_type::nothing: 
        {
            return placement_mode::free_mode;
        }; break;
        

        // hovering over a wall segment, start dragging the segment
        case hover_type::wall_segment:
        {
            return placement_mode::dragging_wall_segment;
        } break;    
    
    }
    // default state, do nothing
    return placement_mode::idle_mode;
}

EXPORT_GAME_STATE(LevelEditor, gStateLevelEditor);
