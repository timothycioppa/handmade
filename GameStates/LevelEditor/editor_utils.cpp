#include "editor_utils.hpp"
#include <fstream>
#include <iostream>

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
    stream << "lights"<< std::endl;
    stream << "count 3" << std::endl;
    stream << "light \n col 1 0 0 \n  int .1 \n pos -20 0 0 \n yaw 0 \n pitch 0 \n near 0.01 \n far 100 \n fwidth 20 \n  fheight 20" <<std::endl;
    stream << "light \n col 1 0 0 \n  int .1 \n pos -20 0 0 \n yaw 0 \n pitch 0 \n near 0.01 \n far 100 \n fwidth 20 \n  fheight 20" <<std::endl;
    stream << "light \n col 1 0 0 \n  int .1 \n pos -20 0 0 \n yaw 0 \n pitch 0 \n near 0.01 \n far 100 \n fwidth 20 \n  fheight 20" <<std::endl;

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