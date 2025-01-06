#include "bsp.hpp"
#include <stdio.h>
#include "include/glm/gtc/matrix_transform.hpp"
#include "math_utils.hpp"
#include "light.hpp"
#include <assert.h>

void initialize_solid_wall(wall_segment & segment, node_render_data & wall, bsp_tree & tree) ;
void initialize_segmented_wall( wall_segment & segment,  node_render_data & topWall, node_render_data & bottomWall, bsp_tree & tree) ;
void initialize_sector_floor(sector & s, node_render_data & floor, bsp_tree & tree) ;
void initialize_sector_ceiling(sector & s, node_render_data & ceiling, bsp_tree & tree) ;
unsigned int required_renderables(wall_segment & segment);
insertion_point findInsertionPoint(wall_segment & segment, bsp_node * currentNode, bsp_tree & tree);

wall_segment *get_wall_segment(SectorSide side, sector & s, bsp_tree & tree)
{   
    wall_segment * result = nullptr;  

    #define CHECK_SIDE(side) if (side > -1) { result = &tree.segments[side]; } 

    switch (side) 
    {
        case SectorSide::BOTTOM: { CHECK_SIDE(s.botID) } break;
        case SectorSide::LEFT:   {  CHECK_SIDE(s.leftID)} break;
        case SectorSide::TOP:    { CHECK_SIDE(s.topID) } break;
        case SectorSide::RIGHT:  {  CHECK_SIDE(s.rightID)} break;
    }
    return result;
}

sector* get_sector_recursive(const glm::vec3 & testPos, bsp_node * node, bsp_tree & tree) 
{   
    wall_segment & seg = EXTRACT_WALL_SEGMENT(node);
    float test = glm::dot(seg.normal, testPos - seg.start);

    if (test > 0.0f) 
    {
        if (node->front != nullptr)
        {
            return get_sector_recursive(testPos, node->front, tree);
        }
        else
        {
            if (seg.frontSectorID > -1) 
            {
                sector & sect = tree.sectors[seg.frontSectorID];

                if (aabb_walls_contains(testPos, sect.boundingBox))
                {
                    return &sect;
                }
            }

            if (seg.backSectorID > -1) 
            {
                sector & sect = tree.sectors[seg.backSectorID];
                if (aabb_walls_contains(testPos, sect.boundingBox))
                {
                    return &sect;
                }
            }
       }
    } 
    else 
    {         
        if (node->back != nullptr)
        {
            return get_sector_recursive(testPos, node->back, tree);
        } 
        else
        {
            if (seg.frontSectorID > -1) 
            {
                AABB & frontBB = tree.sectors[seg.frontSectorID].boundingBox;    

                if (aabb_walls_contains(testPos, frontBB))
                {
                    return &tree.sectors[seg.frontSectorID];
                }
            }
            
            if (seg.backSectorID > -1) 
            {
                AABB & backBB = tree.sectors[seg.backSectorID].boundingBox;    
                if (aabb_walls_contains(testPos, backBB))
                {
                    return &tree.sectors[seg.backSectorID];
                }
            }
        }
    } 
    return nullptr;
}

sector* get_sector(const glm::vec3 & testPos, bsp_tree & tree) 
{
    return get_sector_recursive(testPos, tree.root, tree);
}

unsigned int initialize_wall_nodes(bsp_tree & tree) 
{
    int segmentID = 0;
    bsp_node & root = tree.nodes[0];     
    INIT_SEGMENT(root, segmentID)
    tree.root = &root;
    unsigned int requiredRenderables = required_renderables(tree.segments[segmentID]);
    tree.numNodes = 1;

    // create one new bsp_node for each wall segment in the room.
    for (segmentID = 1; segmentID < tree.numSegments; segmentID++) 
    { 
        wall_segment & segment = tree.segments[segmentID];
        bsp_node & newSegmentNode = tree.nodes[segmentID];
        INIT_SEGMENT(newSegmentNode, segmentID)
        insertion_point pivot = findInsertionPoint(segment, tree.root, tree);

        if (pivot.insertionSide == 0) 
        {  
            pivot.node->front = &newSegmentNode;
        }

        if (pivot.insertionSide == 1) 
        {
            pivot.node->back = &newSegmentNode;
        }
        
        requiredRenderables += required_renderables(segment);
        tree.numNodes++;
    }

    return requiredRenderables;

}

unsigned int initialize_wall_renderables(bsp_tree & tree) 
{ 
    unsigned int currentRenderable = 0;

    for (int segIndex = 0; segIndex < tree.numSegments; segIndex++) 
    {
        wall_segment & segment = tree.segments[segIndex];

        // solid wall
        if (segment.backSectorID < 0) 
        {
            node_render_data & wall = tree.renderables[currentRenderable];
            wall.renderFlags = 0;
            wall.type = RenderableType::RT_SOLID_WALL;
            segment.renderIndices.renderableIndex0 = currentRenderable;
            currentRenderable += 1;     
            tree.numRenderables += 1;
            wall.featureIndex = segIndex;
            initialize_solid_wall(segment, wall, tree);
        } 
        else 
        { 
            node_render_data & topWall = tree.renderables[currentRenderable];
            topWall.type = RenderableType::RT_WALL_TOP_SEGMENT;
            topWall.renderFlags = 0;
            segment.renderIndices.renderableIndex0 = currentRenderable;
            
            node_render_data & bottomWall = tree.renderables[currentRenderable + 1]; 
            bottomWall.type = RenderableType::RT_WALL_BOTTOM_SEGMENT;
            bottomWall.renderFlags = 0;
            segment.renderIndices.renderableIndex1 = currentRenderable + 1;

            currentRenderable += 2;            
            tree.numRenderables += 2;

            initialize_segmented_wall(segment, topWall, bottomWall, tree);
                                        
            topWall.featureIndex = segIndex;
            bottomWall.featureIndex = segIndex;
        }

    }    

    return currentRenderable;

}

void initialize_room_renderables(unsigned int currentRenderable, bsp_tree & tree) 
{
    for (int sectorIndex = 0; sectorIndex < tree.numSectors; sectorIndex++) 
    {
        sector & s = tree.sectors[sectorIndex];

        node_render_data & floor = tree.renderables[currentRenderable];
        floor.type = RenderableType::RT_FLOOR;
        floor.renderFlags = 0;
        s.renderIndices.renderableIndex0 = currentRenderable;
        floor.featureIndex = sectorIndex;

        node_render_data & ceiling = tree.renderables[currentRenderable + 1];
        ceiling.type = RenderableType::RT_CEILING;
        ceiling.renderFlags = 0;
        ceiling.featureIndex = sectorIndex;

        s.renderIndices.renderableIndex1 = currentRenderable + 1;
        currentRenderable += 2;               

        initialize_sector_floor(s, floor, tree);
        initialize_sector_ceiling(s, ceiling, tree);           

        tree.numRenderables += 2;
    }
}

void initialize_scene_lights(bsp_tree & tree)
{
 for (int i = 0; i < tree.lightCount; i++) 
    {
        light & l = tree.lights[i];
        l.WorldUp = {0.0f, 1.0f, 0.0f};
        update_light_direction(l);
        update_projection_matrix(l);
    }
}

void build_bsp_tree(bsp_tree & tree)
{  
    // generate all nodes for wall segments, returning the amount of renderables required
    unsigned int requiredRenderables = initialize_wall_nodes(tree);

    requiredRenderables += (2 * tree.numSectors);
    tree.renderables = (node_render_data *) malloc(requiredRenderables * sizeof(node_render_data));
    tree.numRenderables = 0;

    // the tree data is now built. We next go through and generate rendering data for each wall (potentially split), ceiling, and floor node
    unsigned int currentRenderable = initialize_wall_renderables(tree);

    initialize_room_renderables(currentRenderable, tree);

    initialize_scene_lights(tree);   
}

void bsp_tree_free(bsp_tree & tree) 
{
    free(tree.renderables);
    free(tree.segments);
    free(tree.sectors);
    free(tree.lights);
}

void initialize_solid_wall(wall_segment & segment, node_render_data & wall, bsp_tree & tree) 
{
    sector & frontSector = tree.sectors[segment.frontSectorID];
    float wallWidth = glm::distance(segment.start, segment.end);
    float wallHeight = (frontSector.ceilingHeight - frontSector.floorHeight);
    
    wall.transform.position = segment.start + 0.5f * (segment.end - segment.start);
    wall.transform.position.y = frontSector.floorHeight + 0.5f * wallHeight;
    wall.transform.scale = {wallWidth, wallHeight, 1.0f};
    wall.transform.axis = glm::cross(segment.normal, glm::vec3(0,1,0));
    wall.transform.rotation = glm::radians(90.0f);

    // THIS IS VERY HACKY!!!! please don't do this shit, use the axis and rotation from
    // the only reason we do this is to make walls face the right direction. FIXXXX
    glm::vec3 origin = wall.transform.position;
    glm::vec3 lookAt = origin + segment.normal;
    glm::vec3 up = {0,1,0};


    wall.transform.localToWorld = glm::scale(glm::mat4(1.0f),{wallWidth, wallHeight, 1.0f});
    wall.transform.localToWorld = glm::inverse(glm::lookAt(origin, lookAt, up)) * wall.transform.localToWorld;
    wall.transform.dirty = false;

    wall.material.diffuse = {1,1,1};
    wall.material.specular = {1,1,1};
    wall.material.shininess = 1.0f;
    wall.material.mainTexture = nullptr;

}

void initialize_segmented_wall(
    wall_segment & segment, 
    node_render_data & topWall, 
    node_render_data & bottomWall, 
    bsp_tree & tree) 
{ 
    sector & frontSector = tree.sectors[segment.frontSectorID];
    sector & backSector = tree.sectors[segment.backSectorID];

    float wallWidth = glm::distance(segment.start, segment.end);
    float topStart = glm::min(frontSector.ceilingHeight, backSector.ceilingHeight);
    float topEnd = glm::max(frontSector.ceilingHeight, backSector.ceilingHeight);
    float topHeight = topEnd - topStart;

    float bottomStart = glm::min(frontSector.floorHeight, backSector.floorHeight);
    float bottomEnd = glm::max(frontSector.floorHeight, backSector.floorHeight);
    float bottomHeight = bottomEnd - bottomStart;

    glm::vec3 normal = segment.normal;

    if (frontSector.ceilingHeight < backSector.ceilingHeight)
    {
        normal = -normal;
    }

    topWall.material.mainTexture = nullptr;
    topWall.material.diffuse = {1,1,1};
    topWall.material.specular = {1,1,1};
    topWall.material.shininess = 1.0f;

    topWall.transform.position = segment.start + 0.5f * (segment.end - segment.start);
    topWall.transform.position.y = topStart + topHeight * 0.5f;

    // THIS IS VERY HACKY!!!! please don't do this shit, use the axis and rotation from
    // the only reason we do this is to make walls face the right direction. FIXXXX
    glm::vec3 origin = topWall.transform.position;
    glm::vec3 lookAt = origin + normal;
    glm::vec3 up = {0,1,0};

    topWall.transform.localToWorld = glm::scale(glm::mat4(1.0f),{wallWidth, topHeight, 1.0f});
    topWall.transform.localToWorld = glm::inverse(glm::lookAt(origin, lookAt, up)) * topWall.transform.localToWorld;
    topWall.transform.dirty = false;


    normal = segment.normal;

    if (frontSector.floorHeight > backSector.floorHeight)
    {
        normal = -normal;
    }

    bottomWall.material.mainTexture = nullptr;
    bottomWall.material.diffuse = {1,1,1};
    bottomWall.material.specular = {1,1,1};
    bottomWall.material.shininess = 1.0f;

    bottomWall.transform.position = segment.start + 0.5f * (segment.end - segment.start);
    bottomWall.transform.position.y = bottomStart + bottomHeight * 0.5f;

    // THIS IS VERY HACKY!!!! please don't do this shit, use the axis and rotation from
    // the only reason we do this is to make walls face the right direction. FIXXXX
    origin = bottomWall.transform.position;
    lookAt = origin + normal;
    up = {0,1,0};

    bottomWall.transform.localToWorld = glm::scale(glm::mat4(1.0f),{wallWidth, bottomHeight, 1.0f});
    bottomWall.transform.localToWorld = glm::inverse(glm::lookAt(origin, lookAt, up)) * bottomWall.transform.localToWorld;
    bottomWall.transform.dirty = false;
}

void initialize_sector_floor(sector & s, node_render_data & floor, bsp_tree & tree) 
{
    floor.transform.position = {s.center.x, s.floorHeight, s.center.y};
    floor.transform.scale = {s.width, s.height, 1.0f};
    floor.transform.axis = {1,0,0};
    floor.transform.rotation = glm::radians(-180.0f);
    floor.transform.MarkDirty();

    floor.material.diffuse = {1,1,1};
    floor.material.specular = {1,1,1};
    floor.material.shininess = 1.0f;
    floor.material.mainTexture = nullptr;

    glm::vec3 origin = floor.transform.position;
    glm::vec3 lookAt = origin + glm::vec3(0,1,0);
    glm::vec3 up = {0,0,1};

    floor.transform.localToWorld = glm::scale(glm::mat4(1.0f),{s.width, s.height, 1.0f});
    floor.transform.localToWorld = glm::inverse(glm::lookAt(origin, lookAt, up)) * floor.transform.localToWorld;
    floor.transform.dirty = false;

}

void initialize_sector_ceiling(sector & s, node_render_data & ceiling, bsp_tree & tree) 
{
    ceiling.transform.position = glm::vec3(s.center.x, s.ceilingHeight, s.center.y);
    ceiling.transform.scale = {s.width,s.height, 1};
    ceiling.transform.axis = {1,0,0};
    ceiling.transform.rotation = glm::radians(0.0f);
    ceiling.transform.MarkDirty();

    ceiling.material.diffuse = {1,1,1};
    ceiling.material.specular = {1,1,1};
    ceiling.material.shininess = 1.0f;
    ceiling.material.mainTexture = nullptr;

    glm::vec3 origin = ceiling.transform.position;
    glm::vec3 lookAt = origin + glm::vec3(0,-1,0);
    glm::vec3 up = {0,0,1};

    ceiling.transform.localToWorld = glm::scale(glm::mat4(1.0f),{s.width, s.height, 1.0f});
    ceiling.transform.localToWorld = glm::inverse(glm::lookAt(origin, lookAt, up)) * ceiling.transform.localToWorld;
    ceiling.transform.dirty = false;

}

unsigned int required_renderables(wall_segment & segment) 
{
    if (segment.backSectorID > -1) 
    {
        return 2;
    }
    return 1;
}

insertion_point findInsertionPoint(wall_segment & segment, bsp_node * currentNode, bsp_tree & tree) 
{ 
    wall_segment & parentSegment = tree.segments[currentNode->segmentIndex];    
    
    // segment lies in front of the current node's segment
    if (glm::dot(segment.start - parentSegment.start, parentSegment.normal) >= 0 && 
        glm::dot(segment.end - parentSegment.start, parentSegment.normal) >= 0)  
    {
        if (currentNode->front != nullptr) 
        {
            return findInsertionPoint(segment, currentNode->front, tree);
        } else
        {
            return {currentNode, 0};
        } 
    }
    // segment lies behind the current segment
    else
    {
        if (currentNode->back != nullptr) 
        {
            return findInsertionPoint(segment, currentNode->back, tree);
        } else
        {
            return {currentNode, 1};
        } 
    }
}

