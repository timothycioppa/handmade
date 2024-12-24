#ifndef _SCENEOBJECT_HPP
#define _SCENEOBJECT_HPP

#include "Transform.hpp"
#include <string>
#include "Graphics/GLMesh.hpp"
#include <iostream>
#include "TextureStore.hpp"
#include "Material.hpp"

struct scene_object
{ 
    bool enabled;
    Transform transform;
    static_mesh mesh;
    Material material;
    std::string Name;
};

void initialize_mesh(std::string, scene_object &) ;
void release(scene_object) ;


#endif