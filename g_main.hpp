#ifndef _G_MAIN_H
#define _G_MAIN_H
#include "Graphics/GLShader.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "game_data.hpp"
#include "SceneObject.hpp"
#include "TextureStore.hpp"

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

extern shader_depthPass depthPass;
extern depth_Uniforms depthUniforms;
extern shader_fullscreenBlit fullScreenBlit;
extern screen_blit_uniforms blitUniforms;

extern LightData lightData;
extern unsigned int colorBufferUniformID;
extern TextureStore gTextureRepository;

void G_Init();
void G_RenderDepth();
void G_RenderColorBuffer();
void G_RenderFinalFrame();
void G_StartFrame();
void G_Cleanup();
void G_RenderSceneShadowedFull();
void G_RenderOverlay();

#endif