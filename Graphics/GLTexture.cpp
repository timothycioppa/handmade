#include "GLTexture.hpp"
#define STB_IMAGE_IMPLEMENTAION
#include "../include/stb_image.h"

void load_texture(const char* filename, texture_info & tex) { 

    int width, height, channels;
    unsigned char * data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

    if (data != NULL) 
    { 
        glGenTextures(1, &(tex.textureID));
        glBindTexture(GL_TEXTURE_2D, tex.textureID);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);                	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        stbi_image_free(data);
    }            
}

void release_texture(texture_info & tex) 
{ 
    glDeleteTextures(1, &(tex.textureID));
}
