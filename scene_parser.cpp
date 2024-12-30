#include "scene_parser.hpp"
#include "g_main.hpp"
#include "light.hpp"
using namespace std;

void load_scene(const char* filename, scene_data & scene) 
{ 
    fstream stream;
    stream.open(filename);
    std::string soKey, key;

    int count = 0;

    while (true)
    { 
        stream >> key;
        
        if (stream.eof()) 
        { 
            break;
        }

        if (key == "WALL") 
        { 
            room_wall wall;
            stream >> soKey >> wall.wall_plane.origin.x >> wall.wall_plane.origin.y >> wall.wall_plane.origin.z;
            stream >> soKey >> wall.wall_plane.normal.x >> wall.wall_plane.normal.y >> wall.wall_plane.normal.z;
            stream >> soKey >> wall.width >> wall.height;
            scene.room_walls.push_back(wall);    
        }
        
        if (key == "LIGHT") 
        {
            light l;
            stream >> soKey >> l.Color.x >> l.Color.y >> l.Color.z;
            stream >> soKey >> l.intensity;
            stream >> soKey >> l.Position.x >> l.Position.y >> l.Position.z;
            stream >> soKey >> l.Yaw;
            stream >> soKey >> l.Pitch;
            stream >> soKey >> l.near;
            stream >> soKey >> l.far;
            stream >> soKey >> l.frustrumWidth;
            stream >> soKey >> l.frustrumHeight;
            scene.lights.push_back(l);
        }

        if (key == "FLOOR") 
        {
            room_floor floor;
            stream >> soKey >> floor.origin.x >> floor.origin.y >> floor.origin.z;
            stream >> soKey >> floor.width >> floor.height;
            scene.floor = floor; 
        }

        if (key == "CEILING") 
        {
            room_ceiling ceiling;
            stream >> soKey >> ceiling.origin.x >> ceiling.origin.y >> ceiling.origin.z;
            stream >> soKey >> ceiling.width >> ceiling.height;
            scene.ceiling = ceiling; 
        }
    } 

    stream.close();

// initialize floor
    scene_object floor_so;
    floor_so.enabled = true;
    floor_so.Name = "floor";
    Transform & floorTransform = floor_so.transform;
    floorTransform.position = scene.floor.origin;
    floorTransform.scale = {scene.floor.width, 1.0f, scene.floor.height};
    floorTransform.axis = {1,0,0};
    floorTransform.rotation = glm::radians(-180.0f);
    floorTransform.MarkDirty();

    Material & mat = floor_so.material;
    mat.mainTexture = gTextureRepository.GetTexture("Textures/uvtemplate.bmp");
    mat.diffuse =  {1.0f, 1.0f, 1.0f};
    mat.specular =  {1.0f, 1.0f, 1.0f};
    mat.shininess =   1.0f;
    scene.sceneObjects.push_back(floor_so);

// initialize ceiling

    scene_object ceiling_so;
    ceiling_so.enabled = true;
    ceiling_so.Name = "ceiling";
    Transform & ceilingTransform = ceiling_so.transform;
    ceilingTransform.position = scene.ceiling.origin;
    ceilingTransform.scale = {scene.ceiling.width, 1.0f, scene.ceiling.height};
    ceilingTransform.axis = {1,0,0};
    ceilingTransform.rotation = glm::radians(0.0f);
    ceilingTransform.MarkDirty();

    Material & c_mat = ceiling_so.material;
    c_mat.mainTexture = gTextureRepository.GetTexture("Textures/uvtemplate.bmp");
    c_mat.diffuse =  {1.0f, 1.0f, 1.0f};
    c_mat.specular =  {1.0f, 1.0f, 1.0f};
    c_mat.shininess =   100.0f;
    scene.sceneObjects.push_back(ceiling_so);

    for (room_wall wall : scene.room_walls) 
    { 
        scene_object so;
        so.enabled = true;
        so.Name = "test object";

        Transform & debugTransform = so.transform;
        debugTransform.position = wall.wall_plane.origin;
        debugTransform.scale = {wall.width, 1.0f, wall.height};
        debugTransform.axis = glm::cross(wall.wall_plane.normal, glm::vec3(0,1,0));
        debugTransform.rotation = glm::radians(90.0f);
        debugTransform.MarkDirty();
        
        Material & material = so.material;
        material.mainTexture = gTextureRepository.GetTexture("Textures/arabesque.bmp");
        material.diffuse =  {1.0f, 1.0f, 1.0f};
        material.specular =  {1.0f, 1.0f, 1.0f};
        material.shininess =   1.0f;

        scene.sceneObjects.push_back(so);
    }

    for (scene_object & so : scene.sceneObjects) 
    {
        initialize_mesh("Models/quad.obj", so);
    }

    for (light & l : scene.lights) 
    {
        l.WorldUp = {0.0f, 1.0f, 0.0f};
        update_light_direction(l);
        update_projection_matrix(l);
    }

}