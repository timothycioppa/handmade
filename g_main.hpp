#ifndef _G_MAIN_H
#define _G_MAIN_H
#include "Graphics/GLShader.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "game_data.hpp"
#include "scene_object.hpp"
#include "TextureStore.hpp"
#include "scene_parser.hpp"

struct LightData 
{
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 lightSpaceMatrix;
	float near;
	float far;
};

struct FrameBufferInfo 
{ 
	unsigned int fbo;
	unsigned int texture;
};

extern float lightStrength;

extern shader_depthPass shadowDepthPass;
extern depth_Uniforms shadowDepthUniforms;

extern shader_hdr_blit fullScreenHDRBlit;
extern hdr_uniforms hdrBlitUniforms;

extern LightData lightData;
extern unsigned int colorBufferUniformID;
extern TextureStore gTextureRepository;

void G_Init();
void G_RenderShadowDepth(scene_data&);
void G_RenderToHDRColorBuffer( scene_data&);
void G_RenderFinalFrame();
void G_StartFrame();
void G_RenderSceneShadowedFull(scene_data&);
void G_RenderLevelEditor();
void G_RenderOverlay();
void G_Cleanup();
void debug_line(glm::vec3, glm::vec3, glm::vec3, camera_data&);

#endif