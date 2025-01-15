#ifndef _G_MAIN_H
#define _G_MAIN_H
#include "Graphics/GLShader.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "game_data.hpp"
#include "TextureStore.hpp"
#include "scene_parser.hpp"
#include "bsp.hpp"
#include "GameStates/LevelEditor/editor_render_context.hpp"
#include "GameStates/Gameplay/gameplay_context.hpp"

struct LightData 
{
	glm::vec3 position;
	glm::vec3 color;
	float intensity;	

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
	unsigned int depthTexture;
	unsigned int normalTexture;
	unsigned int positionTexture;
};

extern LightData shadowLight;

void G_Init();
void G_RenderFinalFrame();
void G_StartFrame();
void G_RenderShadowDepth(bsp_tree&);
void G_RenderScene(bsp_tree&);
void G_RenderSceneShadowedFull(bsp_tree&, gameplay_context&);
void G_RenderProjectile(glm::vec3 position, bsp_tree & scene);
void G_RenderTitleScreen();
void G_RenderLevelEditor(editor_render_context & renderContext) ;
void G_RenderOverlay();
void G_Cleanup();
void debug_line(glm::vec3, glm::vec3, glm::vec3, camera_data&);
void ValidateTextures(bsp_tree & tree) ;

#endif