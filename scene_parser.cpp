#include "scene_parser.hpp"
#include "g_main.hpp"
#include "light.hpp"
#include "bsp.hpp"
using namespace std;


// dummy variables used when parsing the scene file
std::string key;
std::string dummy;


// MEMORY ARENA - all scene memory is allocated from this arena
#define SCENE_ARENA_SIZE 1024*1024
unsigned char gSceneMemory[SCENE_ARENA_SIZE];
unsigned char* gMemoryHead;
unsigned char* allocateFromArena(int numBytes) ;
#define ARENA_ALLOC(N, T) (T*) allocateFromArena((N) * sizeof(T))


void load_sector_data(bsp_tree & tree, fstream & stream);
void load_segment_data(bsp_tree & tree, fstream & stream);
void load_node_data(bsp_tree & tree, fstream & stream);
void loadSector(int index, bsp_tree & tree, fstream & stream);
void loadSegment(int index, bsp_tree & tree, fstream & stream);
void loadNode(int index, bsp_tree & tree, fstream & stream);
unsigned int _required_renderables(wall_segment & segment) ;
unsigned int _calculate_required_renderables(bsp_tree & tree);


void load_scene(const char* filename, bsp_tree & tree) 
{    
    gMemoryHead = &gSceneMemory[0];

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
            tree.sectors = ARENA_ALLOC(tree.numSectors, sector);
            load_sector_data(tree, stream);

        }


        if (key == "segments")
        {
            stream >> dummy >> tree.numSegments;
            tree.segments = ARENA_ALLOC(tree.numSegments, wall_segment);
            load_segment_data(tree, stream);
        }     


        if (key == "nodes")
        {   
            stream >> dummy >> tree.numNodes; 
            tree.nodes = ARENA_ALLOC(tree.numNodes, bsp_node);
            load_node_data(tree, stream);
            tree.root = &tree.nodes[0]; 
        }     

    }

    stream.close();
    unsigned int requiredRenderables = _calculate_required_renderables(tree); // either 1 or 2 renderables per wall segment
    requiredRenderables += (2 * tree.numSectors); // 2 renderables (floor and ceiling) for each sector
    tree.renderables = ARENA_ALLOC(requiredRenderables, node_render_data);  
    
    std::cout << "memory used for scene data: [" << (gMemoryHead - gSceneMemory) << " bytes] " <<std::endl; 
}

void load_sector_data(bsp_tree & tree, fstream & stream) {
    for (int i = 0; i < tree.numSectors; i++) {
        loadSector(i, tree, stream);
    }
}

void load_segment_data(bsp_tree & tree, fstream & stream) {
     for (int i = 0; i < tree.numSegments; i++) {
        loadSegment(i, tree, stream);
    }
}

void load_node_data(bsp_tree & tree, fstream & stream) 
{
    for (int i = 0; i < tree.numNodes; i++) 
    {
        loadNode(i, tree, stream);
    }
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

    s.normal = segment_normal(s);
    
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

    if (frontID > -1) 
    {
        node.front = &tree.nodes[frontID]; 
    } else 
    {
        node.front = nullptr;
    }

    if (backID > -1) 
    {
        node.back = &tree.nodes[backID]; 
    } else
    {
        node.back = nullptr;
    }
}


unsigned int _required_renderables(wall_segment & segment) 
{
    if (segment.backSectorID > -1) 
    {
        return 2;
    }
    return 1;
}


unsigned int _calculate_required_renderables(bsp_tree & tree) 
{
    unsigned int requiredRenderables = 0;

    // create one new bsp_node for each wall segment in the room.
    for (int segmentID = 0; segmentID < tree.numSegments; segmentID++) 
    { 
        wall_segment & segment = tree.segments[segmentID];
        bsp_node & newSegmentNode = tree.nodes[segmentID];
        requiredRenderables += _required_renderables(segment);
    }

    return requiredRenderables;
}

unsigned char* allocateFromArena(int numBytes) 
{
    unsigned char* result = gMemoryHead;
    gMemoryHead += numBytes;
    return result;
}