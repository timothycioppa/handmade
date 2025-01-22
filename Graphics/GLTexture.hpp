#ifndef _GL_TEXTURE_HPP
#define _GL_TEXTURE_HPP

#include "../utils.hpp"

struct texture_info 
{ 
    unsigned int textureID;
    unsigned int width;
    unsigned int height; 
};

void load_bmp(const char* filename, texture_info & tex); 
void load_texture(const char* filename, texture_info & tex);
void release_texture(texture_info & tex);

#endif