#include "bsp_collision.hpp"

bool test_pos_bsp(const glm::vec3 & testPos, bsp_node * node, bsp_tree & tree) 
{ 
    wall_segment & seg = tree.segments[node->segmentIndex];
    float test = glm::dot(seg.normal, testPos - seg.start);

    if (test > 0.0f) 
    {
        if (node->front != nullptr)
        {
            return test_pos_bsp(testPos, node->front, tree);
        } else
        {
            sector & s = tree.sectors[ seg.frontSectorID];

            if (testPos.y >= s.floorHeight && testPos.y + 2.0f <= s.ceilingHeight) 
            {
                return true;
            }

            return false;
        }
    } 
    else 
    {         
        if (node->back != nullptr)
        {
            return test_pos_bsp(testPos, node->back, tree);
        } else
        {
            sector & s = tree.sectors[seg.backSectorID];

            if (testPos.y >= s.floorHeight && testPos.y + 2.0f <= s.ceilingHeight) 
            {
                return true;
            }

            return false;
        }
    } 
}

bool test_pos_bsp(const glm::vec3 & testPos, bsp_tree & tree) 
{ 
    return test_pos_bsp(testPos, tree.root, tree);
}

bool try_get_intersection(const ray_t & ray, const AABB & bb, boxIntersection & intersection)
{
    static glm::vec3 normals[6] = 
    {
        glm::vec3(0,1,0),
        glm::vec3(0,-1,0),
        glm::vec3(1,0,0),
        glm::vec3(-1, 0, 0),
        glm::vec3(0,0,1),
        glm::vec3(0,0,-1)
    };

    glm::vec3 up = glm::vec3(0, bb.extents.y, 0);
    glm::vec3 forward = glm::vec3(0,0,bb.extents.z);
    glm::vec3 right = glm::vec3(bb.extents.x, 0, 0);
    
    static plane_t planes[6];

    planes[0] =     {bb.center + up, normals[0] };
    planes[1] =      {bb.center - up, normals[1] };
    planes[2] =     {bb.center + right, normals[2] };
    planes[3] =      {bb.center - right, normals[3] };
    planes[4] =     {bb.center + forward, normals[4] };
    planes[5] =      {bb.center - forward, normals[5] };
       
    float minDist = 999999.0f;
    int index = -1;

    for (int i = 0; i < 6; i++)
    {            
        float dist = -1.0f;
        
        if (distance_to_plane(ray, planes[i], &dist))
        {
            if (dist >= 0)
            {
                if (dist < minDist)
                {
                    minDist = dist;
                    index = i;
                }
            }
        }

    }

    // actually hit a side of the wall
    if (index > -1)
    {
        intersection.point = ray.origin + minDist * ray.direction;
        intersection.side = (BoxSide) index;
        return aabb_contains(intersection.point, bb);
    }

    return false;
}


bool try_get_WALL_intersection(const ray_t & ray, const AABB & bb, boxIntersection & intersection)
{
    static glm::vec3 normals[6] = 
    {
        glm::vec3(0,1,0),
        glm::vec3(0,-1,0),
        glm::vec3(1,0,0),
        glm::vec3(-1, 0, 0),
        glm::vec3(0,0,1),
        glm::vec3(0,0,-1)
    };

    glm::vec3 up = glm::vec3(0, bb.extents.y, 0);
    glm::vec3 forward = glm::vec3(0,0,bb.extents.z);
    glm::vec3 right = glm::vec3(bb.extents.x, 0, 0);
    
    static plane_t planes[6];

    planes[0] =     {bb.center + up, normals[0] };
    planes[1] =      {bb.center - up, normals[1] };
    planes[2] =     {bb.center + right, normals[2] };
    planes[3] =      {bb.center - right, normals[3] };
    planes[4] =     {bb.center + forward, normals[4] };
    planes[5] =      {bb.center - forward, normals[5] };
       
    float minDist = 999999.0f;
    int index = -1;

    for (int i = 2; i < 6; i++)
    {            
        float dist = -1.0f;
        
        if (distance_to_plane(ray, planes[i], &dist))
        {
            if (dist >= 0)
            {
                if (dist < minDist)
                {
                    minDist = dist;
                    index = i;
                }
            }
        }

    }

    // actually hit a side of the wall
    if (index > -1)
    {
        intersection.point = ray.origin + minDist * ray.direction;
        intersection.side = (BoxSide) index;
        return aabb_contains(intersection.point, bb);
    }

    return false;
}


plane_t get_ceiling_plane(const sector &s)
{
    plane_t result;
    result.origin = {s.center.x, s.ceilingHeight, s.center.y};
    result.normal = {0,1,0};
    return result;
}

plane_t get_floor_plane(const sector & s){
    plane_t result;
    result.origin = {s.center.x, s.floorHeight, s.center.y};
    result.normal = {0,-1,0};
    return result;
}

bool try_get_intersection(const ray_t & ray, const plane_t & plane, planeIntersection & intersection) 
{
    float dist =  glm::dot(plane.normal, plane.origin - ray.origin) / glm::dot(plane.normal, ray.direction);
    
    if (dist >= 0)
    {
        intersection.point = ray.origin + dist * ray.direction;
        return true;
    }

    return false;
}

bool bsp_raycast(glm::vec3 origin, glm::vec3 direction, raycast_hit & hit, bsp_tree & tree) 
{
    sector* previousSector = get_sector(origin, tree);
    sector* currentSector = previousSector;
    glm::vec3 current_pos = origin;
    float step_size = 0.5f;
    int iterations = 0;
    ray_t ray = {origin, direction};
    glm::vec3 shifted_origin = current_pos;

    while (true) 
    {

        if (iterations > MAX_RAYCAST_ITERATIONS) 
        {
            return false;
        }

        current_pos += (step_size * direction); 
        currentSector = get_sector(current_pos, tree);
    
        // we've passed into a new sector!
        if(currentSector != previousSector)
        {
            // passed into a new sector but maybe passed the floor/ceiling
            if (currentSector != nullptr)
            {
                shifted_origin = current_pos;
                AABB & boundingBox = currentSector->boundingBox;
                                
                if (!aabb_contains(current_pos, boundingBox))
                {                                        
                    boxIntersection outIntersection;
                    
                    // NOTE: CAREFUL! we're just assuming that we'll get an intersection point! 
                    try_get_WALL_intersection(ray, boundingBox, outIntersection);

                    if (outIntersection.point.y > currentSector->ceilingHeight) 
                    {
                        hit.RenderType = RenderableType::RT_WALL_TOP_SEGMENT;
                    }

                    if (outIntersection.point.y < currentSector->floorHeight)
                    {
                        hit.RenderType = RenderableType::RT_WALL_BOTTOM_SEGMENT;
                    }

                    hit.hitSector = currentSector;
                    hit.hitSegment = nullptr;
                    hit.position = outIntersection.point;
                    hit.distance = glm::distance(origin, outIntersection.point);                    

                    int segmentID = -1;
                    
                    switch(outIntersection.side) 
                    {
                        case BoxSide::BS_LEFT:      { segmentID = currentSector->leftID;} break;
                        case BoxSide::BS_FRONT:     { segmentID = currentSector->topID; } break;
                        case BoxSide::BS_RIGHT:     { segmentID = currentSector->rightID;} break;
                        case BoxSide::BS_BACK:      { segmentID = currentSector->botID;} break;
                    };

                    if (segmentID > -1)
                    {
                        hit.hitSegment = &tree.segments[segmentID];
                    }
                    return true;
                }    
            }
            else 
            // new sector is null, hit a solid wall
            {
                if (previousSector != nullptr)
                {
                    AABB &boundingBox = previousSector->boundingBox;
                    boxIntersection outIntersection;

                    // NOTE: we use the shifted origin to do the ray test to make sure we can raycast to
                    // walls that are in adjacent segments. 
                    if (try_get_intersection({shifted_origin, direction}, boundingBox, outIntersection))
                    {
                        hit.hitSector = previousSector;
                        hit.hitSegment = nullptr;
                        hit.position = outIntersection.point;
                        hit.distance = glm::distance(origin, outIntersection.point);    
                        bool ceiling = false, floor = false;

                        int segmentID = -1;

                        switch(outIntersection.side) 
                        {
                            case BoxSide::BS_LEFT:      { segmentID = previousSector->leftID;} break;
                            case BoxSide::BS_FRONT:     { segmentID = previousSector->topID; } break;
                            case BoxSide::BS_RIGHT:     { segmentID = previousSector->rightID;} break;
                            case BoxSide::BS_BACK:      { segmentID = previousSector->botID;} break;
                        };

                        if ( segmentID > -1) 
                        {
                            hit.hitSegment = &tree.segments[segmentID];
                            hit.RenderType = RenderableType::RT_SOLID_WALL;
                        }
                        return true;                             
                    }
                    return false;
                }
            }
        } 
        else
        {
            // still in the same sector
            if (currentSector != nullptr)
            {
                const sector & curr = *currentSector;

                // hit the ceiling
                if (current_pos.y > curr.ceilingHeight) 
                {
                    plane_t ceilingPlane = get_ceiling_plane(curr);
                    planeIntersection outIntersection;

                    if (try_get_intersection(ray, ceilingPlane, outIntersection))
                    {
                        hit.RenderType = RenderableType::RT_CEILING;
                        hit.hitSector = currentSector;
                        hit.hitSegment = nullptr;
                        hit.position = outIntersection.point;
                        hit.distance = glm::distance(origin, outIntersection.point);    
    
                        return true;                             
                    }

                    return false;     
                }   

                // hit the floor
                if (current_pos.y < curr.floorHeight)
                {  
                    plane_t floorPlane = get_floor_plane(curr);
                    planeIntersection outIntersection;

                    if (try_get_intersection(ray, floorPlane, outIntersection))
                    {                   
                        hit.RenderType = RenderableType::RT_FLOOR;
                        hit.hitSector = currentSector;
                        hit.hitSegment = nullptr;
                        hit.position = outIntersection.point;
                        hit.distance = glm::distance(origin, outIntersection.point);    
                        return true;                             
                    }

                    return false;     
                }       
            }
        }

        previousSector = currentSector;
        iterations++;             
    }

    return false;
}
