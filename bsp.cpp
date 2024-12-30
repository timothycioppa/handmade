#include "bsp.hpp"
#include <stdio.h>

sector* get_sector_recursive(const glm::vec3 & testPos, bsp_node * node, bsp_tree & tree) 
{
    // in an open sector
    if (node->sector_id > -1) 
    {
        return &tree.sectors[node->sector_id];
    } 
    else 
    { 
        // in a closed sector
        if (node->segment_id < 0)
        { 
            return nullptr;
        }
    }

    wall_segment & seg = tree.segments[node->segment_id];
    glm::vec3 center = seg.start + 0.5f * (seg.end - seg.start);

    float test = glm::dot(seg.normal, testPos - center);

    if (test > 0.0f) 
    {
        return get_sector_recursive(testPos, node->front, tree);
    } 
    else 
    {         
        return get_sector_recursive(testPos, node->back, tree);
    } 
}

sector* get_sector(const glm::vec3 & testPos, bsp_tree & tree) 
{
    return get_sector_recursive(testPos, tree.root, tree);
}

bool test_pos_bsp(const glm::vec3 & testPos, bsp_tree & tree) 
{ 
    return test_pos_bsp(testPos, tree.root, tree);
}

bool test_pos_bsp(const glm::vec3 & testPos, bsp_node * node, bsp_tree & tree) 
{ 

     // in an open sector
    if (node->sector_id > -1) 
    {
        return true;
    } 
    else 
    { 
        // in a closed sector
        if (node->segment_id < 0)
        { 
            return false;
        }
    }

    wall_segment & seg = tree.segments[node->segment_id];
    glm::vec3 center = seg.start + 0.5f * (seg.end - seg.start);

    float test = glm::dot(seg.normal, testPos - center);

    if (test > 0.0f) 
    {
        return test_pos_bsp(testPos, node->front, tree);
    } 
    else 
    {         
        return test_pos_bsp(testPos, node->back, tree);
    } 
}

bool test_pos_bsp(const glm::vec3 & testPos, bsp_node * node, bsp_node * parent, bsp_node ** failed_node, bsp_tree & tree) 
{ 
   // in an open sector
    if (node->sector_id > -1) 
    {
        *failed_node = nullptr;
        return true;
    } 
    else 
    { 
        // in a closed sector
        if (node->segment_id < 0)
        { 
            *failed_node = parent;
            return false;
        }
    }


    wall_segment & seg = tree.segments[node->segment_id];
    glm::vec3 center = seg.start + 0.5f * (seg.end - seg.start);
    float test = glm::dot(seg.normal, testPos - center);

    if (test > 0.0f) 
    {
        return test_pos_bsp(testPos, node->front, node, failed_node, tree);
    } 
    else 
    {         
        return test_pos_bsp(testPos, node->back, node, failed_node, tree);
    } 
    
}


bool bsp_raycast(glm::vec3 origin, glm::vec3 direction, raycast_hit & hit, bsp_tree & tree) 
{
    bsp_node * failed_node = nullptr;
    glm::vec3 current_pos = origin;
    glm::vec3 previous_pos = origin;
    float step_size = 0.5f;
    int count = 0;

    while (test_pos_bsp(current_pos, tree.root, nullptr, &failed_node, tree)) 
    {
       previous_pos = current_pos;
       current_pos += (step_size * direction); 
       count++;

       if (count > MAX_RAYCAST_ITERATIONS) 
       {
            printf("too many iterations\n");
            return false;
       }
    }

    if (failed_node != nullptr) 
    {
        wall_segment & crossed_segment = tree.segments[failed_node->segment_id];
        glm::vec3 normal = crossed_segment.normal;
        float d = glm::dot(normal, crossed_segment.start - origin) / dot(normal, direction);
        hit.position = origin + d * direction;
        return true;
    }

    return false;
}


void bsp_tree_free(bsp_tree & tree) 
{
    free (tree.nodes);
    free(tree.segments);
}