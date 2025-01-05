#include "scene_parser.hpp"
#include "g_main.hpp"
#include "light.hpp"
#include "bsp.hpp"
using namespace std;

void load_scene(const char* filename, bsp_tree & tree) 
{    
    fstream stream;
    stream.open(filename);
    std::string soKey, key;

    int count = 0;
    int currentLight = 0;

    int maxLines = 100;

    while (true)
    { 
        stream >> key;
        
        maxLines--;
        if (maxLines < 0)
        {
            printf("too many lines\n");
            break;
        }

        if (stream.eof()) 
        { 
            break;
        }

        if (key == "sectors") 
        { 

            stream >> soKey >> tree.numSectors;
            tree.sectors = (sector*) malloc(tree.numSectors * sizeof(sector));

            for (int i = 0; i < tree.numSectors; i++) 
            {
                sector & s = tree.sectors[i];

                stream >> soKey >> s.floorHeight;       
                stream >> soKey >> s.ceilingHeight;
                stream >> soKey >> s.center.x >> s.center.y;
                stream >> soKey >> s.width >> s.height;
                stream >> soKey >> s.botID >> s.leftID >> s.topID >> s.rightID;
                s.initialized = false;
                s.renderIndices = {-1, -1};
                
                s.boundingBox.center = {s.center.x, s.floorHeight + 0.5f * (s.ceilingHeight - s.floorHeight), s.center.y};
                s.boundingBox.extents = {0.5f * s.width, 0.5f * (s.ceilingHeight - s.floorHeight), 0.5f * s.height };
            }
        }

        if (key == "segments")
        {
            stream >> soKey >> tree.numSegments;
            tree.segments = (wall_segment*) malloc(tree.numSegments * sizeof(wall_segment));

            for (int i = 0; i < tree.numSegments; i++) 
            {
                wall_segment & s = tree.segments[i];
                stream >> soKey >> s.start.x >> s.start.z;
                stream >> soKey >> s.end.x >> s.end.z;
                s.start.y = 0.0f;
                s.end.y = 0.0f;

                s.normal = glm::cross(glm::normalize(s.start - s.end), glm::vec3(0,1,0));

                int frontID, backID;
                stream >> soKey >> frontID;
                stream >> soKey >> backID;

                s.frontSectorID = frontID;
                s.backSectorID = backID;                   
                s.renderIndices = {-1, -1};
            }
        }

        if (key == "lights") 
        {
            stream >> soKey >> tree.lightCount;
            tree.lights = (light*) malloc(tree.lightCount * sizeof(light));
        }

        if (key == "light") 
        {
            light & l = tree.lights[currentLight];
            stream >> soKey >> l.Color.x >> l.Color.y >> l.Color.z;
            stream >> soKey >> l.intensity;
            stream >> soKey >> l.Position.x >> l.Position.y >> l.Position.z;
            stream >> soKey >> l.Yaw;
            stream >> soKey >> l.Pitch;
            stream >> soKey >> l.near;
            stream >> soKey >> l.far;
            stream >> soKey >> l.frustrumWidth;
            stream >> soKey >> l.frustrumHeight;
            currentLight++;
        }
    }

    stream.close();
 }
