#ifndef _TEXT_REANDERER_H
#define _TEXT_REANDERER_H

#include "../include/glm/gtc/matrix_transform.hpp"
#include "GLShader.hpp"
#include <map>
#include <stb_image.h>
#include <ft2build.h>
#include <iostream>
#include "../../ShaderStore.hpp"
#include FT_FREETYPE_H  

/// Holds all state information relevant to a character as loaded using FreeType
struct Character 
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

struct FontInfo 
{ 
    GLuint _VAO, _VBO;
    std::map<GLchar, Character> Characters;
    glm::mat4 projection;
};

 bool InitializeFont(std::string font_name, int width, int height,  FontInfo * textData) ;

void DrawText(std::string text, float x, float y, float scale, glm::vec3 color, FontInfo * textData)  ;

#endif