#ifndef EDITOR_UTILS_HPP
#define EDITOR_UTILS_HPP

#include <stdio.h>
#include "../../platform_common.hpp"
#include "../../g_main.hpp"
#include "../../editor_controller.hpp"

#define PV2(v) printf("(%g %g)\n", v.x, v.y)
#define PV3(v) printf("(%g %g %g)\n", v.x, v.y, v.z)
#define PV4(v) printf("(%g %g %g %g)\n", v.x, v.y, v.z, v.w)

#define MAX_EDITOR_SECTORS 100
#define MAX_EDITOR_SEGMENTS 400

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
    dragging_wall_segment = 1,
    idle_mode = 2
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
    float width;
    float height;
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

struct editor_scene_data 
{
    int numSectors;
    int numSegments;
    sector_e sectors[MAX_EDITOR_SECTORS];
    segment_e segments[MAX_EDITOR_SEGMENTS];
};

typedef void (*PlacementModeCallback) (placement_data*);

struct placement_mode_state
{
    PlacementModeCallback OnStart;
    PlacementModeCallback OnUpdate;
    PlacementModeCallback OnEnd;
};

#define PLACEMENT_STATE_FUNC(name) void name(placement_data* pData)

void exportScene(editor_scene_data & scene, const char* filename) ;
void renderDebugBox(sector_e &box) ;
void render_sector(sector_e & box, editor_scene_data & scene) ;
placement_mode getCurrentPlacementMode(hover_type type);
glm::vec3 getWorldCoordinates(glm::vec2 & windowCoordinates, editor_render_context & renderContext) ;
bool checkHoverPosition(glm::vec3 worldPosition, sector_e & box, int sectorIndex, hover_data & hover);

#endif