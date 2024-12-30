#ifndef _BSP_HPP
#define _BSP_HPP

#include "platform_common.hpp"

#define FILLED 0
#define EMPTY 1
#define SPLIT 2

#define MAX_RAYCAST_ITERATIONS 100

struct wall_segment 
{ 
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 normal;
};

struct sector 
{ 
    float floorHeight;
    float ceilingHeight;
};

struct bsp_node 
{
    // the sector we're in. This should be -1 unless we're at a leaf corresponding to an open sector.  
    int sector_id;

    // the id of the segment dividing the scene. This should be -1 except for interior nodes of the bsp tree.
    int segment_id;

    // pointers to the front/back of the above segment. These should be null except for interior nodes.
    bsp_node * back;
    bsp_node * front;
};

struct bsp_tree 
{    
    // represents a line segment dividing the scene
    unsigned int numSegments;
    wall_segment *segments;
    
    // represent open rooms the player can go into
    unsigned int numSectors;
    sector* sectors;

    unsigned int numNodes;
    bsp_node *nodes;   


    bsp_node *root;    
};

struct raycast_hit 
{ 
    glm::vec3 position;
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
bool test_pos_bsp(const glm::vec3 & testPos, bsp_tree & tree);
bool test_pos_bsp(const glm::vec3 & testPos, bsp_node * node, bsp_tree & tree) ;
bool test_pos_bsp(const glm::vec3 & testPos, bsp_node * node, bsp_node * parent, bsp_node ** failed_node, bsp_tree & tree) ;
bool bsp_raycast(glm::vec3 origin, glm::vec3 direction, raycast_hit & hit, bsp_tree & tree) ;
void bsp_tree_free(bsp_tree & tree);

#endif
