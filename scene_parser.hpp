#ifndef _SCENE_PARSER_HPP
#define _SCENE_PARSER_HPP

#include "math_utils.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include "SceneObject.hpp"

struct scene_data 
{ 
    std::vector<room_wall> room_walls;
    std::vector<SceneObject> sceneObjects;
};

void load_scene(const char* filename, scene_data & scene);

#endif