#ifndef _BSP_HPP
#define _BSP_HPP

#include "platform_common.hpp"
#include "light.hpp"
#include "Material.hpp"
#include "math_utils.hpp"
#define MAX_RAYCAST_ITERATIONS 256

enum NodeType 
{ 
    WALL_SEGMENT,
    SECTOR
};

struct feature_index 
{
    NodeType Type;
    int ID; // index of either segment or sector (depending on above type)
};

// each node can render up to 2 things (floor + ceiling or top + bottom wall segment)
struct renderable_index 
{
    int renderableIndex0; // index of render data, either floor (if Type == sector), or a wall
    int renderableIndex1; // index of either ceiling or 
};

struct bsp_node 
{  
    feature_index featureIndex;
    bsp_node * front;
    bsp_node * back;
};

#define FEATURE_TYPE(node) (node)->featureIndex.Type
#define FEATURE_INDEX(node) (node)->featureIndex.ID
#define MAX_TREE_NODES 256
#define RENDERABLE_FEATURE renderable_index renderIndices; 

struct sector 
{
    RENDERABLE_FEATURE

    float floorHeight;
    float ceilingHeight;
    float width;
    float height;
    glm::vec2 center;    
    bool initialized;

    // cached for collision detection speedup
    AABB boundingBox;

    // pointers to segments comprising this sector. MUST BE IN THIS ORDER IN THE SAVE FILE
    int botID;
    int leftID;
    int topID;
    int rightID;
};

enum SectorSide 
{ 
    BOTTOM, 
    LEFT, 
    TOP,
    RIGHT
};

struct wall_segment
{
    RENDERABLE_FEATURE
    
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 normal;

    int frontSectorID; // should never be null
    int backSectorID; // null for solid walls    
};

enum RenderableType
{
    RT_FLOOR,
    RT_CEILING,
    RT_SOLID_WALL,
    RT_WALL_TOP_SEGMENT,
    RT_WALL_BOTTOM_SEGMENT
};

enum RenderFlags 
{
    FLAG_0 = (1 << 0),
    FLAG_1   = (1 << 1),
    FLAG_2 = (1 << 2),
    FLAG_3 = (1 << 3)
};


struct node_render_data 
{
    bool highlighted;
    unsigned int renderFlags;    
    RenderableType type; 
    feature_index indexData;
    Transform transform;
    Material material;
};

#define IS_HIGHLIGHTED(r) ((((r).renderFlags) & FLAG_0) > 0)
#define SET_HIGHLIGHTED(r) (((r).renderFlags) |= FLAG_0)
#define UNSET_HIGHLIGHTED(r) (((r).renderFlags) &= ~FLAG_0)

struct bsp_tree 
{    
    // represents a line segment dividing the scene
    unsigned int numSegments;
    wall_segment *segments;

    // represent open rooms the player can go into
    unsigned int numSectors;
    sector* sectors;

    // all render data for the entire scene geometry
    unsigned int numRenderables;
    node_render_data *renderables;

    // all scene lights. somewhat innapropriate but fine for now
    unsigned int lightCount;
    light* lights;

    // bsp tree structure. Traverse down through sectors (in front/behind) until we reach a room (or null if outside bounds of room)
    unsigned int numNodes;
    bsp_node nodes[MAX_TREE_NODES];       
    bsp_node *root;    

};

struct raycast_hit 
{ 
    RenderableType RenderType;
    sector *hitSector;
    wall_segment *hitSegment;
    float distance;
    glm::vec3 position;
};

struct insertion_point
{
    bsp_node * node;
    int insertionSide; // 0 = front, 1 = back
};


#define SECTOR_NODE(s, sid) s->sector_id = sid; \
    s->segment_id = -1; \
    s->front = s->back = nullptr; \

#define FILLED_NODE(s) s->sector_id = -1; \
    s->segment_id = -1; \
    s->front = s->back = nullptr; \

#define SPLIT_NODE(s, i, b, f) s->sector_id = -1; \
        s->segment_id = i; \
        s->back = b;  \
        s->front = f; \
         
sector* get_sector(const glm::vec3 & testPos, bsp_tree & tree);
void bsp_tree_free(bsp_tree & tree);
void build_bsp_tree(bsp_tree & tree);
renderable_index get_render_indices(bsp_node * node, bsp_tree tree);
wall_segment *get_wall_segment(SectorSide side, sector & s, bsp_tree & tree);


#define INIT_SEGMENT(n, i) n.featureIndex.Type = NodeType::WALL_SEGMENT; n.featureIndex.ID = i; n.front = n.back = nullptr;
#define INIT_SECTOR(n, i) n.featureIndex.Type = NodeType::SECTOR; n.featureIndex.ID = i; n.front = n.back = nullptr;
#define EXTRACT_SECTOR(n) tree.sectors[FEATURE_INDEX(n)]
#define EXTRACT_WALL_SEGMENT(n) tree.segments[FEATURE_INDEX(n)]

#endif
