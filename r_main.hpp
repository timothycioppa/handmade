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
#include "scene_object.hpp"
#include "scene_parser.hpp"

extern unsigned int quadVAO;
extern unsigned int quadVBO;

extern shader_coloredRect coloredRect;
extern coloredrect_uniforms coloredRectUniforms;

extern shader_texturedRect texturedRect;
extern texrect_uniforms texRectUniforms;

extern shader_shadowed standardShadowed;
extern shadowed_uniforms shadowedUniforms;

enum GameFont 
{ 
    Ariel,
    Anton
};

void R_Init();
void R_Cleanup() ;
void R_RenderFullScreenQuad() ;
void R_DrawColoredRect(glm::vec3 ll, glm::vec3 d, glm::vec3 c) ;
void R_DrawTexturedRect (glm::vec3 ll, glm::vec3 d, texture_info* texture);
void R_RenderMeshStandardShadowed(scene_data & scene,  RenderContext & context);
void R_DrawText(std::string text, float x, float y, float scale, glm::vec3 color, GameFont font);  

#endif