#include "scene_parser.hpp"
#include "g_main.hpp"
#include "light.hpp"
#include "bsp.hpp"
using namespace std;

char key[64];
char dummy[64];

void loadSector(int index, bsp_tree & tree, fstream & stream);
void loadSegment(int index, bsp_tree & tree, fstream & stream);
void loadNode(int index, bsp_tree & tree, fstream & stream);

void load_scene(const char* filename, bsp_tree & tree) 
{    
    fstream stream;
    stream.open(filename);
    int maxLines = 1000;

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

            stream >> dummy >> tree.numSectors;
            tree.sectors = (sector*) malloc(tree.numSectors * sizeof(sector));

            for (int i = 0; i < tree.numSectors; i++) 
            {
                loadSector(i, tree, stream);
            }
        }

        if (key == "segments")
        {
            stream >> dummy >> tree.numSegments;
            tree.segments = (wall_segment*) malloc(tree.numSegments * sizeof(wall_segment));

            for (int i = 0; i < tree.numSegments; i++) 
            {
               loadSegment(i, tree, stream);
            }
        }     


        // NOTE(josel): don't rebuild the tree structure, just load it in from the scene file like below

        // if (key == "nodes")
        // {
        //     stream >> soKey >> tree.numNodes;
        //     tree.nodes = (bsp_node*) malloc(tree.numNodes * sizeof(bsp_node));

        //     for (int i = 0; i < tree.numNodes; i++) 
        //     {
        //         bsp_node & node = tree.nodes[i];
        //         stream >> soKey >> node.segmentIndex;
        //         int frontID, backID;
        //         stream >> soKey >> frontID;
        //         stream >> soKey >> backID;
        //         node.front = &tree.nodes[frontID]; 
        //         node.back = &tree.nodes[backID]; 
        //     }

        //     tree.root = &tree.nodes[0];
        // }     
    }

    stream.close();
 }


void loadSector(int index, bsp_tree & tree, fstream & stream)
{
    sector & s = tree.sectors[index];
    stream >> dummy >> s.floorHeight;       
    stream >> dummy >> s.ceilingHeight;
    stream >> dummy >> s.center.x >> s.center.y;
    stream >> dummy >> s.width >> s.height;
    stream >> dummy >> s.botID >> s.leftID >> s.topID >> s.rightID;
    s.initialized = false;
    s.renderIndices = {-1, -1};
    s.boundingBox.center = {s.center.x, s.floorHeight + 0.5f * (s.ceilingHeight - s.floorHeight), s.center.y};
    s.boundingBox.extents = {0.5f * s.width, 0.5f * (s.ceilingHeight - s.floorHeight), 0.5f * s.height };
}

void loadSegment(int index, bsp_tree & tree, fstream & stream)
{
    wall_segment & s = tree.segments[index];
    stream >> dummy >> s.start.x >> s.start.z;
    stream >> dummy >> s.end.x >> s.end.z;
    s.start.y = 0.0f;
    s.end.y = 0.0f;

    s.normal = glm::cross(glm::normalize(s.start - s.end), glm::vec3(0,1,0));

    int frontID, backID;
    stream >> dummy >> frontID;
    stream >> dummy >> backID;

    s.frontSectorID = frontID;
    s.backSectorID = backID;                   
    s.renderIndices = {-1, -1};
}

void loadNode(int index, bsp_tree & tree, fstream & stream) 
{
    bsp_node & node = tree.nodes[index];
    stream >> dummy >> node.segmentIndex;
    int frontID, backID;
    stream >> dummy >> frontID;
    stream >> dummy >> backID;
    node.front = &tree.nodes[frontID]; 
    node.back = &tree.nodes[backID]; 
}