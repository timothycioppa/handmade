#ifndef _R_MAIN_HPP
#define _R_MAIN_HPP

#include "Graphics/GLShader.hpp"
#include "Graphics/GLTexture.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "game_data.hpp"
#include "Material.hpp"
#include "Graphics/GLRenderContext.hpp"
#include "Graphics/GLMesh.hpp"
#include "Graphics/GLTextRenderer.hpp"
#include "scene_parser.hpp"
#include "game_state.hpp"

#define FONT_ENTRY(index, enum_code, path) enum_code = index,
enum GameFont 
{ 
    #include "font_entries.hpp"
};
#undef FONT_ENTRY

void R_Init(void);
void R_Cleanup(void);
void R_RenderFullScreenQuad(void);
void R_DrawColoredRect(glm::vec3 ll, glm::vec3 d, glm::vec3 c) ;
void R_DrawTexturedRect (glm::vec3 ll, glm::vec3 d, texture_info* texture);
void R_DrawText(const char* text, float x, float y, float scale, glm::vec3 color, GameFont font);  
void R_DrawLines(void);
void R_DrawLine(glm::vec2 start, glm::vec2 end, glm::vec3 color); 
void R_DrawMeshInstanced(static_mesh & m, unsigned int count) ;
void R_DrawMesh(static_mesh & m) ;
void R_DrawAllText(void); 

#endif