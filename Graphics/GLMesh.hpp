#ifndef _GLMESH_HPP
#define _GLMESH_HPP

#include "../utils.hpp"
#include "../platform_common.hpp"
#include <vector>

struct static_mesh 
{ 
        GLuint VAO, VBO, NBO, UVBO; 
        unsigned int vertexCount;
};

void load_mesh(const char* fileName, static_mesh & m) ;
void render_mesh(static_mesh & m);
void release_mesh (static_mesh & m) ;

#endif