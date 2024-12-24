#include "scene_parser.hpp"
#include "g_main.hpp"
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
        std::cout << key << std::endl;
        
        std::cout << count << std::endl;
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
        count++;
        if (count == 4) { 
            break;
        }
    } 

    stream.close();

    for (room_wall wall : scene.room_walls) 
    { 
        SceneObject so;
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

    for (SceneObject & so : scene.sceneObjects) 
    {
        so.initialize_mesh("Models/quad.obj");
    }

}