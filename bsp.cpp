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
unsigned int required_renderables(wall_segment & segment) ;
insertion_point findInsertionPoint(wall_segment & segment, bsp_node * currentNode, bsp_tree & tree);

wall_segment *get_wall_segment(SectorSide side, sector & s, bsp_tree & tree)
{   
    wall_segment * result = nullptr;
   
    switch (side) 
    {
        case SectorSide::BOTTOM: { if (s.botID > -1) {  result = &tree.segments[s.botID]; } } break;
        case SectorSide::LEFT: { if (s.leftID > -1) {  result = &tree.segments[s.leftID]; } } break;
        case SectorSide::TOP: { if (s.topID > -1) {  result = &tree.segments[s.topID]; } } break;
        case SectorSide::RIGHT: { if (s.rightID > -1) {  result = &tree.segments[s.rightID]; } } break;
    }

    return result;
}

renderable_index get_render_indices(bsp_node * node, bsp_tree tree)
{
    renderable_index result = {-1, -1};
    
    if (node != nullptr)
    {
        NodeType nodeType = FEATURE_TYPE(node);
        int ID = FEATURE_INDEX(node);

        switch (nodeType) 
        {
            case NodeType::SECTOR: 
            { 
                sector & s = tree.sectors[ID];
                result.renderableIndex0 = s.renderIndices.renderableIndex0;
                result.renderableIndex1 = s.renderIndices.renderableIndex1;
            }  break;

            case NodeType::WALL_SEGMENT: 
            { 
                wall_segment & s = tree.segments[ID];
                result.renderableIndex0 = s.renderIndices.renderableIndex0;
                result.renderableIndex1 = s.renderIndices.renderableIndex1;
            }  break; 
        }

    }

    return result;
}

sector* get_sector_recursive(const glm::vec3 & testPos, bsp_node * node, bsp_tree & tree) 
{
    if (node == nullptr) 
    {
        return nullptr;
    }

    // in an open sector
    if (FEATURE_TYPE(node) == NodeType::SECTOR) 
    {
        return &tree.sectors[FEATURE_INDEX(node)];
    } 
   
    wall_segment & seg = tree.segments[ FEATURE_INDEX(node)];
    float test = glm::dot(seg.normal, testPos - seg.start);

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

bsp_node* find_sector_node(int sectorID, bsp_tree & tree)
{
    if (sectorID > -1)
    {
        for (int i = 0; i < tree.numNodes; i++)
        {
            if (tree.nodes[i].featureIndex.Type == NodeType::SECTOR && tree.nodes[i].featureIndex.ID == sectorID)
            {
                return &tree.nodes[i];
            }
        }
    }
    return nullptr;
}

void build_bsp_tree(bsp_tree & tree)
{
    
    int requiredRenderables = 0;

    // hard initialize the root (just to make finding pivot points easier)    
    int segmentID = 0;

    bsp_node & root = tree.nodes[0];     
    INIT_SEGMENT(root, segmentID)
    tree.root = &root;
    requiredRenderables += required_renderables(tree.segments[segmentID]);
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

    printf("nodes %d\n", tree.numNodes);

    // render data for floor and ceiling (for each sector)
    requiredRenderables += (2 * tree.numSectors);
    
    // we should have tree.numNodes new nodes in the list.
    // we will now insert fewer (or the same number) nodes, corresponding to the sectors in the room
    int nodeCount = tree.numNodes;

    for (int i = 0; i < nodeCount; i++) 
    {
        bsp_node & segmentNode = tree.nodes[i];
        wall_segment & segment = tree.segments[FEATURE_INDEX(&segmentNode)];

        printf("segment [%d]\n", i);
        printf("\t sector ids [%d, %d]\n", segment.backSectorID, segment.frontSectorID);
        printf("\t pointer    [%d, %d]\n", segmentNode.back == nullptr, segmentNode.front == nullptr);
        
        if (segmentNode.back == nullptr)
        {
            if (segment.backSectorID > -1) 
            {
                if (!tree.sectors[segment.backSectorID].initialized)
                {
                    tree.sectors[segment.backSectorID].initialized = true;
                    bsp_node & newSectorNode = tree.nodes[tree.numNodes];                
                    INIT_SECTOR(newSectorNode, segment.backSectorID) 
                    tree.numNodes++;        
                    segmentNode.back = find_sector_node(segment.backSectorID, tree);
                } 
            }
        }

        if (segmentNode.front == nullptr)
        {
            if (segment.frontSectorID > -1) 
            {
                if (!tree.sectors[segment.frontSectorID].initialized)
                {
                    tree.sectors[segment.frontSectorID].initialized = true;                    
                    bsp_node & newSectorNode = tree.nodes[tree.numNodes];
                    INIT_SECTOR(newSectorNode, segment.frontSectorID)                    
                    tree.numNodes++;        
                    segmentNode.front = find_sector_node(segment.frontSectorID, tree);   
                } 
            }       
        }
    }


    // each sector requires a floor and a ceiling quad
    tree.renderables = (node_render_data *) malloc(requiredRenderables * sizeof(node_render_data));
    tree.numRenderables = 0;

    // the tree data is now built. We next go through and generate rendering data for each wall (potentially split), ceiling, and floor node
    int currentRenderable = 0;

    for (int i = 0; i < tree.numNodes; i++) 
    {
        bsp_node & node = tree.nodes[i];
        bsp_node *pNode = &node;

        switch (FEATURE_TYPE(pNode)) 
        {
            case NodeType::SECTOR: 
            { 
                sector & s = tree.sectors[FEATURE_INDEX(pNode)];


                node_render_data & floor = tree.renderables[currentRenderable];
                floor.type = RenderableType::RT_FLOOR;
                floor.renderFlags = 0;
                s.renderIndices.renderableIndex0 = currentRenderable;
                floor.indexData = {NodeType::SECTOR, FEATURE_INDEX(pNode)};


                node_render_data & ceiling = tree.renderables[currentRenderable + 1];
                ceiling.type = RenderableType::RT_CEILING;
                ceiling.renderFlags = 0;
                ceiling.indexData = {NodeType::SECTOR, FEATURE_INDEX(pNode)};

                s.renderIndices.renderableIndex1 = currentRenderable + 1;
                currentRenderable += 2;               


                initialize_sector_floor(s, floor, tree);
                initialize_sector_ceiling(s, ceiling, tree);           

                tree.numRenderables += 2;

            } break;

            case NodeType::WALL_SEGMENT: 
            { 
                wall_segment & segment = tree.segments[FEATURE_INDEX(pNode)];

                // solid wall
                if (segment.backSectorID < 0) 
                {
                    node_render_data & wall = tree.renderables[currentRenderable];
                    wall.renderFlags = 0;
                    wall.type = RenderableType::RT_SOLID_WALL;
                    segment.renderIndices.renderableIndex0 = currentRenderable;

                    currentRenderable += 1;     
                    tree.numRenderables += 1;

                    wall.indexData = {NodeType::WALL_SEGMENT, FEATURE_INDEX(pNode)};

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
                                                    
                    topWall.indexData = {NodeType::WALL_SEGMENT, FEATURE_INDEX(pNode)};
                    bottomWall.indexData = {NodeType::WALL_SEGMENT, FEATURE_INDEX(pNode)};

                }
            } break;
        };
    }    

    for (int i = 0; i < tree.lightCount; i++) 
    {
        light & l = tree.lights[i];
        l.WorldUp = {0.0f, 1.0f, 0.0f};
        update_light_direction(l);
        update_projection_matrix(l);
    }
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
    printf("SOLID WALL\n");

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

    // MVP = p * v * m;
    // localToProjection * worldToLocal * localToWorld

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

    
    printf("SEGMENTED WALL\n");
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
    printf("FLOOR\n");


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
    printf("CEILING\n");


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
    wall_segment & parentSegment = tree.segments[FEATURE_INDEX(currentNode)];    
    
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

