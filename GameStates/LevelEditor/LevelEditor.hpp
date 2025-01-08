#ifndef _LEVEL_EDITOR_HPP
#define _LEVEL_EDITOR_HPP

#include "../game_state.hpp"
#include "../game_context.hpp"
#include "../g_main.hpp"
#include "../editor_controller.hpp"

#define PV2(v) printf("(%g %g)\n", v.x, v.y)
#define PV3(v) printf("(%g %g %g)\n", v.x, v.y, v.z)
#define PV4(v) printf("(%g %g %g %g)\n", v.x, v.y, v.z, v.w)

enum hover_type 
{ 
    wall_segment, 
    wall_corner,
    nothing
};

enum sector_side 
{ 
    ss_bot,
    ss_left,
    ss_top,
    ss_right
};

struct hover_data 
{ 
    hover_type type;
    int sectorIndex;
    sector_side sectorSide;
};

enum placement_mode 
{ 
    free_mode = 0,
    dragging_wall_segment = 1
};

struct placement_data 
{
    glm::vec3 currentWorldPosition;
    int sectorIndex;
    sector_side sectorSide;
};

// editor sector
struct sector_e 
{ 
    glm::vec3 startCorner;
    glm::vec3 endCorner;
    glm::vec3 botColor, leftColor, topColor, rightColor;
    int botSegmentID, leftSegmentID, topSegmentID, rightSegmentID;
};

// editor segment
struct segment_e 
{ 
    glm::vec3 start;
    glm::vec3 end;
    int frontSectorID;
    int backSectorID;
};


typedef void (*PlacementModeCallback) (placement_data*);

struct placement_mode_state
{
    PlacementModeCallback OnStart;
    PlacementModeCallback OnUpdate;
    PlacementModeCallback OnEnd;
};

extern game_state gStateLevelEditor;

GAMESTATE_INIT(LevelEditor);
GAMESTATE_UPDATE(LevelEditor);
GAMESTATE_RENDER(LevelEditor);
GAMESTATE_POSTRENDER(LevelEditor);
GAMESTATE_EDITOR(LevelEditor);
GAMESTATE_DESTROY(LevelEditor);

#endif