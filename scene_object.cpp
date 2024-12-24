#include "scene_object.hpp"


void initialize_mesh(std::string modelSource, scene_object & so) 
{ 
    load_mesh(modelSource.c_str(), so.mesh);
}

void release(scene_object & so) 
{ 
    release_mesh(so.mesh);
}
