#include "GLTexture.hpp"

void load_texture(const char* filename, texture_info & tex) { 
     BMP bmp = loadBMP(filename);

    if (bmp.data != NULL) 
    { 
        glGenTextures(1, &(tex.textureID));

        glBindTexture(GL_TEXTURE_2D, tex.textureID);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.width, bmp.height, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);                	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
    }            
}

void release_texture(texture_info & tex) 
{ 
    glDeleteTextures(1, &(tex.textureID));
}
