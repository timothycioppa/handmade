#include "editor_utils.hpp"
#include <fstream>
#include <iostream>
#include <stdlib.h>

#define INSERT_FRONT 0
#define INSERT_BACK 1


struct insertion_point_e 
{ 
    unsigned int side;
    int parentNodeID;
};

insertion_point_e getInsertionPoint(int parentNodeID, segment_e & thisSegment, editor_scene_data & scene) 
{ 
    bsp_node_e & parentNode = scene.nodes[parentNodeID];
    segment_e parentSegment = scene.segments[parentNode.segment_id];
    glm::vec3 normal = glm::cross(glm::normalize(parentSegment.start - parentSegment.end), {0,1,0}); 
    float test0 = glm::dot(normal, thisSegment.start - parentSegment.start);
    float test1 = glm::dot(normal, thisSegment.end - parentSegment.start);

    // segment is in front of parent segment
    if (test0 >= 0 && test1 >= 0) 
    {
        if (parentNode.frontID > -1)
        {
            return getInsertionPoint(parentNode.frontID, thisSegment, scene);
        } else 
        {
            return { INSERT_FRONT, parentNodeID };
        }
    }
    else
    {
        if (parentNode.backID > -1)
        {
            return getInsertionPoint(parentNode.backID, thisSegment, scene);
        } 
        else 
        {
            return { INSERT_BACK, parentNodeID };
        }
    }
}

void build_bsp_tree(editor_scene_data & scene) 
{
    scene.nodes[0].segment_id = 0;
    scene.nodes[0].backID = -1;
    scene.nodes[0].frontID = -1;

    for (int i = 1; i < scene.numSegments; i++) 
    {
        segment_e & testSegment = scene.segments[i];
        bsp_node_e & node = scene.nodes[i];
        node.segment_id = i;
        node.backID = node.frontID = -1;

        insertion_point_e pivot = getInsertionPoint(0, testSegment, scene);
        bsp_node_e & insertionParent = scene.nodes[pivot.parentNodeID];

        if (pivot.side == INSERT_FRONT) 
        {
            insertionParent.frontID = i;
        }

        if (pivot.side == INSERT_BACK) 
        {
            insertionParent.backID = i;
        }
    }

    scene.numNodes = scene.numSegments;
}

void exportScene(editor_scene_data & scene, const char* filename) 
{ 
    std::ofstream stream(filename);
    stream << "sectors" << std::endl;
    stream << "count " << scene.numSectors << std::endl;

    for (int i = 0; i < scene.numSectors; i++) 
    {
        sector_e & s = scene.sectors[i];
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
    stream << "count " << scene.numSegments << std::endl;

    for (int i = 0; i < scene.numSegments; i++) 
    {
        segment_e & s = scene.segments[i];

        stream
            << "s " << s.start.x << " " << s.start.z  << " "
            << "e " << s.end.x << " " << s.end.z << " "
            << "f " << s.frontSectorID << " " << "b " << s.backSectorID << std::endl;    
    }
    
    stream << "nodes" << std::endl;
    stream << "count " << scene.numNodes << std::endl;

    for (int i = 0; i < scene.numNodes; i++) 
    {
        bsp_node_e node = scene.nodes[i];
        stream << "seg " << node.segment_id << " f " << node.frontID << " b " << node.backID << std::endl; 
    }

    stream.close();
}

void renderDebugBox(sector_e &box) 
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

void render_sector(sector_e & box, editor_scene_data & scene)
{
    segment_e & top = scene.segments[box.topSegmentID];
    segment_e & right = scene.segments[box.rightSegmentID];
    segment_e & bot = scene.segments[box.botSegmentID];
    segment_e & left = scene.segments[box.leftSegmentID];
    
    glm::vec3 normal;

    #define LRP(a,b) (a + 0.5f * (b - a))

    #define RENDER_SEGMENT(s, c) debug_line(s.start, s.end,  c,   gEditorPlayer.camData); \
        normal = glm::cross(glm::normalize(s.start - s.end), glm::vec3(0,1,0)); \
        debug_line(LRP(s.start, s.end), LRP(s.start, s.end) + 5.0f * normal, glm::vec3(0,1,1), gEditorPlayer.camData); \

    RENDER_SEGMENT(top, box.topColor)
    RENDER_SEGMENT(right, box.rightColor)
    RENDER_SEGMENT(bot, box.botColor)
    RENDER_SEGMENT(left, box.leftColor)
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


glm::vec3 getWorldCoordinates(glm::vec2 & windowCoordinates, editor_render_context & renderContext) 
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
        if (abs(worldPosition.z - start.z) < thresh)
        {
            box.botColor = glm::vec3(1,0,0);        
            hover.type = hover_type::wall_segment;
            hover.sectorIndex = sectorIndex;
            hover.sectorSide = sector_side::ss_bot;
            
            return true;
        }
        else if (abs(worldPosition.z - end.z) < thresh)
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
        if (abs(worldPosition.x - start.x) < thresh)
        {
            box.leftColor = glm::vec3(1,0,0);     
            hover.type = hover_type::wall_segment;
            hover.sectorIndex = sectorIndex;
            hover.sectorSide = sector_side::ss_left;


            return true;
        }
        else if (abs(worldPosition.x - end.x) < thresh)
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