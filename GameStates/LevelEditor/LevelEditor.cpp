#include "LevelEditor.hpp"
#include "editor_render_context.hpp"

//glm::mat4 clipToWorld = glm::mat4(1.0f);
sector_e *sectors;
segment_e *segments;

#define MAX_EDITOR_SECTORS 100
#define MAX_EDITOR_SEGMENTS 400

int sectorHead;
int segmentHead;

hover_data hoverData;
placement_mode mode;

// used for creating new sectors
sector_e debug = 
{
    glm::vec3(0.0f), glm::vec3(0.0f),
    glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f)
};

placement_data data;
bool placing = false;

glm::vec3 getWorldCoordinates(glm::vec2 & windowCoordinates);
void clear_hover_data();
bool checkHoverPosition(glm::vec3 worldPosition, sector_e & box, int sectorIndex, hover_data & hover);
void render_box(sector_e & box);
placement_mode getCurrentPlacementMode(hover_type type);

void FreeModeStart(placement_data* pData);
void FreeModeUpdate(placement_data* pData);
void FreeModeEnd(placement_data* pData);
void WallDragStart(placement_data* pData);
void WallDragUpdate(placement_data* pData);
void WallDragEnd(placement_data* pData);

editor_render_context renderContext;

placement_mode_state modeStates[2] = 
{
    {
        FreeModeStart,
        FreeModeUpdate,
        FreeModeEnd
    },

    {
        WallDragStart,
        WallDragUpdate,
        WallDragEnd
    }
};

GAMESTATE_INIT(LevelEditor)
{
	EditorPlayer_Init(&context);
    renderContext.clipToWorld = glm::inverse(gEditorPlayer.camData.projection * gEditorPlayer.camData.view);
    sectors = (sector_e*) malloc(MAX_EDITOR_SECTORS * sizeof(sector_e));
    segments = (segment_e*) malloc(MAX_EDITOR_SEGMENTS * sizeof(segment_e));
    sectorHead = 0;
    segmentHead = 0;
}

GAMESTATE_UPDATE(LevelEditor)
{
    // 1. get world position under cursor
    renderContext.clipToWorld = glm::inverse(gEditorPlayer.camData.projection * gEditorPlayer.camData.view);
    glm::vec3 currentWorldPosition = getWorldCoordinates(gContext.mousePosition); 
    data.currentWorldPosition = currentWorldPosition;

    if (!placing)
    {
        // 2. figure out what we're hovering over
        clear_hover_data();

        for (int i = 0; i < sectorHead; i++) 
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

    if (mouse_button_held(MouseButtons::M_LEFT)) 
    {
        placement_mode_state & state = modeStates[int(mode)];
        state.OnUpdate(&data); 
    }

    if (mouse_button_released(MouseButtons::M_LEFT))
    {
        placing = false;
        placement_mode_state & state = modeStates[int(mode)];
        state.OnEnd(&data);
    }

    if (mouse_button_pressed(MouseButtons::M_LEFT)) 
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

    for (int i = 0; i < sectorHead; i++) 
    { 
        sector_e & curr = sectors[i];
        render_box(curr);
    }

    renderContext.cameraPosition = gEditorPlayer.Position;
    G_RenderLevelEditor(renderContext);
}

GAMESTATE_POSTRENDER(LevelEditor)
{

}

GAMESTATE_EDITOR(LevelEditor)
{}

GAMESTATE_DESTROY(LevelEditor)
{
    free(sectors);
    free(segments);
}

void FreeModeStart(placement_data* pData) 
{ 
    debug.startCorner = pData->currentWorldPosition;
    debug.endCorner = pData->currentWorldPosition;
}

void FreeModeUpdate(placement_data* pData) 
{ 
    debug.endCorner = pData->currentWorldPosition;
}

void FreeModeEnd(placement_data* pData) 
{ 
    // adjust corners to be proper bottom left / top right
    sector_e & newSector = sectors[sectorHead];
    
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

    int sectorIndex = sectorHead;
    sectorHead++;

    segment_e &bot = segments[segmentHead];
    newSector.botSegmentID = segmentHead;
    bot.frontSectorID = sectorIndex;
    bot.backSectorID = -1;    
    bot.start = debug.endCorner;
    bot.start.z = debug.startCorner.z;    
    bot.end = debug.startCorner;


    segment_e &left = segments[segmentHead+1];
    newSector.leftSegmentID = segmentHead + 1;
    left.frontSectorID = sectorIndex;
    left.backSectorID = -1;
    left.start = debug.startCorner;
    left.end = debug.startCorner;
    left.end.z = debug.endCorner.z;


    segment_e &top = segments[segmentHead+2];
    newSector.topSegmentID = segmentHead + 2;
    top.frontSectorID = sectorIndex;
    top.backSectorID = -1;
    top.start = debug.startCorner;
    top.end = debug.endCorner;
    top.start.z = debug.endCorner.z;


    segment_e &right = segments[segmentHead+3];
    newSector.rightSegmentID = segmentHead + 3;
    right.frontSectorID = sectorIndex;
    right.backSectorID = -1;
    right.start = debug.endCorner;
    right.end = debug.endCorner;
    right.end.z = debug.startCorner.z;

    segmentHead += 4;
}

void WallDragStart(placement_data* pData) 
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

void WallDragUpdate(placement_data* pData) 
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

void WallDragEnd(placement_data* pData) 
{ 
    // adjust corners to be proper bottom left / top right
    sector_e & newSector = sectors[sectorHead];
    
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

    sectorHead++;

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

void clear_hover_data() 
{ 
    hoverData.type = hover_type::nothing;
    hoverData.sectorIndex = -1;

    for (int i = 0; i < sectorHead; i++) 
    {
        sector_e & sector = sectors[i];
    
        sector.botColor = glm::vec3(1,1,1);
        sector.leftColor = glm::vec3(1,1,1);
        sector.rightColor = glm::vec3(1,1,1);
        sector.topColor = glm::vec3(1,1,1);
    }
}

bool checkHoverPosition(glm::vec3 worldPosition, sector_e & box, int sectorIndex, hover_data & hover)
{
    glm::vec3 & start = box.startCorner;
    glm::vec3 & end = box.endCorner;
    float thresh = 0.15f;
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
        case hover_type::nothing: return placement_mode::free_mode;
    }
    return placement_mode::dragging_wall_segment;
}

EXPORT_GAME_STATE(LevelEditor, gStateLevelEditor);
