#include "g_main.hpp"
#include "player.hpp"
#include "game_context.hpp"
#include "r_main.hpp"

#include "math_utils.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#include "include/glm/gtc/type_ptr.hpp"
#include "editor_controller.hpp"

float lightStrength = 0.5f;

shader_grid editorGrid;

shader_depthPass shadowDepthPass;
depth_Uniforms shadowDepthUniforms;

shader_hdr_blit fullScreenHDRBlit;
hdr_uniforms hdrBlitUniforms;

FrameBufferInfo shadowDepthMapFB;
FrameBufferInfo hdrColorBufferFB;
LightData lightData;
TextureStore gTextureRepository;

// used for rendering walls and ceilings (since everything is a quad)
static_mesh quadMesh;

bool useHDR = true;
float exposure = 1.0f;

void    GenerateHDRColorBuffer();
void    GenerateShadowDepthBuffer();
void    GenerateLightData();

void G_Init() 
{
    COMPILE_SHADER("Shaders/depth.vert", "Shaders/depth.frag", shadowDepthPass)
    COMPILE_SHADER("Shaders/hdr.vert", "Shaders/hdr.frag", fullScreenHDRBlit);
    COMPILE_SHADER("Shaders/editorGrid.vert", "Shaders/editorGrid.frag", editorGrid);

    load_mesh("Models/wall.obj", quadMesh);
    GenerateHDRColorBuffer();
    GenerateShadowDepthBuffer();
    GenerateLightData();

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void G_Cleanup() 
{ 
    RELEASE_SHADER(shadowDepthPass)
    RELEASE_SHADER(fullScreenHDRBlit);
    release_mesh(quadMesh);

	for (auto & [key, value] : gTextureRepository.TextureMap ) 
	{ 
		release_texture(value);
	}
}

// uses a rendertexture for depth instead of a texture we can access after the HDR pass
void GenerateHDRColorBuffer_OLD() 
{ 
    unsigned int hdrFBO;
    unsigned int colorBuffer;
    unsigned int rboDepth;

    glGenFramebuffers(1, &hdrFBO);
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    
    // attach color and depth textures 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
         
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    hdrColorBufferFB.fbo = hdrFBO;
    hdrColorBufferFB.texture = colorBuffer;
    hdrColorBufferFB.depthTexture = rboDepth;
}

void GenerateHDRColorBuffer() 
{ 
    unsigned int hdrFBO;
    unsigned int colorBuffer;
    unsigned int rboDepth;

    glGenFramebuffers(1, &hdrFBO);
    
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glGenTextures(1, &rboDepth);
    glBindTexture(GL_TEXTURE_2D, rboDepth);
    {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rboDepth, 0);
    }
         
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    hdrColorBufferFB.fbo = hdrFBO;
    hdrColorBufferFB.texture = colorBuffer;
    hdrColorBufferFB.depthTexture = rboDepth;
}

void GenerateShadowDepthBuffer() 
{ 
    glGenFramebuffers(1, &(shadowDepthMapFB.fbo));    
    glGenTextures(1, &(shadowDepthMapFB.texture));
    glBindTexture(GL_TEXTURE_2D, shadowDepthMapFB.texture);
	
    {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowDepthMapFB.fbo);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthMapFB.texture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void ValidateTextures(bsp_tree & tree) 
{
    // set default texture if none is set
    for (int i = 0; i < tree.numRenderables; i++) {
       if (tree.renderables[i].material.mainTexture == nullptr)
       {
            tree.renderables[i].material.mainTexture = gTextureRepository.GetTexture("Textures/freImage1.jpeg");
       } 
    }
}

void GenerateLightData() 
{
    lightData.near = 0.01f;
    lightData.far = 100.0f;
    float halfOrthoWidth = 10.0f;
    float halfOrthoHeight = 10.0f;
    lightData.projection = glm::ortho(-halfOrthoWidth, halfOrthoWidth, -halfOrthoHeight, halfOrthoHeight, lightData.near, lightData.far);
    glm::vec3 center = glm::vec3(5.0f , 5.0f, 5.0f);
	lightData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), center, glm::vec3(0.0, 1.0, 0.0));
	lightData.lightSpaceMatrix = (lightData.projection) * (lightData.view);
 }

void render_shadow_depth_recursive(bsp_node* node, bsp_tree & tree)
{
    if (node->back != nullptr) 
    {
        render_shadow_depth_recursive(node->back, tree);
    }

    if (node->front != nullptr) 
    {
        render_shadow_depth_recursive(node->front, tree);
    }

    renderable_index & indices = tree.segments[node->segmentIndex].renderIndices;

    if (indices.renderableIndex0 > -1)
    {
        glm::mat4 local2LightSpace = 
            tree.lights[0].lightSpace * tree.renderables[indices.renderableIndex0].transform.localToWorldMatrix();             
        glUniformMatrix4fv(shadowDepthPass.uniformsIDS.local2LightSpace, 1, GL_FALSE, glm::value_ptr(local2LightSpace));
        render_mesh(quadMesh);
    }

    if (indices.renderableIndex1 > -1)
    {
        glm::mat4 local2LightSpace = 
            tree.lights[0].lightSpace  * tree.renderables[indices.renderableIndex1].transform.localToWorldMatrix();                        
        glUniformMatrix4fv(shadowDepthPass.uniformsIDS.local2LightSpace, 1, GL_FALSE, glm::value_ptr(local2LightSpace));
        render_mesh(quadMesh);
    }
}

void G_RenderShadowDepth(bsp_tree & scene) 
{ 
    BIND_SHADER(shadowDepthPass)   
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowDepthMapFB.fbo);
    glClear(GL_DEPTH_BUFFER_BIT);	                
    render_shadow_depth_recursive(scene.root, scene);
    unbind_shader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void G_StartFrame() 
{ 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void G_RenderToHDRColorBuffer(bsp_tree & scene) 
{ 
    static RenderContext context;

    context.shadowMapID = shadowDepthMapFB.texture;
    context.cameraPosition = main_player.Position;
    context.cameraForward = main_player.Forward;
    context.lightSpace = lightData.lightSpaceMatrix;
    context.lightPosition = { 0.0f, 0.0f, 0.0f};
    context.lightColor = {1.0f, 1.0f, 0.0f};
    context.lightPower = 2.0f;
    context.totalTime = gContext.applicationTime;
    context.deltaTime = gContext.deltaTime;
    context.sinTime = gContext.sinTime;
    context.cosTime = gContext.cosTime;
    context.v = main_player.camData.view;
    context.p = main_player.camData.projection;

    glBindFramebuffer(GL_FRAMEBUFFER, hdrColorBufferFB.fbo);  
    glClearColor(0,0,0, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    R_RenderMeshStandardShadowed(scene, context); 
}


void G_RenderFinalFrame() 
{
    float width = gContext.windowWidth;
    float height = gContext.windowHeight; 

    glViewport(0, 0, width , height );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    
    BIND_SHADER(fullScreenHDRBlit);
    hdrBlitUniforms.hdrBuffer = hdrColorBufferFB.texture;
    hdrBlitUniforms.hdr = true;
    hdrBlitUniforms.exposure = 1.0;
    set_uniforms(fullScreenHDRBlit, hdrBlitUniforms);
    R_RenderFullScreenQuad();

    unbind_shader();
}

void debug_line (glm::vec3 _start, glm::vec3 _end, glm::vec3 color, camera_data & cam) 
{
    glm::mat4 vp = cam.projection * cam.view;
    glm::vec4 ss =  vp * glm::vec4(_start, 1.0f);
    glm::vec4 ee = vp * glm::vec4(_end, 1.0f);

    ss.x = ss.x / ss.w;
    ss.y = ss.y / ss.w;
    ee.x = ee.x / ee.w;
    ee.y = ee.y / ee.w;

    R_DrawLine(glm::vec2(ss.x, ss.y ), glm::vec2(ee.x , ee.y), color); 
}

glm::vec2 vertCrosshairStart = glm::vec2(0.0f, -0.05f);
glm::vec2 vertCrosshairEnd = glm::vec2(0.0f, 0.05f);
glm::vec2 horCrosshairStart = glm::vec2(-0.05f, 0.0f);
glm::vec2 horCrosshairEnd = glm::vec2(0.05f, 0.0f);
glm::vec3 crossColor = glm::vec3(1,0,0);

void draw_crosshairs() 
{ 
    float ar = gContext.aspectRatio;
    R_DrawLine(horCrosshairStart, horCrosshairEnd, crossColor);
    R_DrawLine(vertCrosshairStart * ar, vertCrosshairEnd * ar, crossColor);    
}

void G_RenderSceneShadowedFull(bsp_tree & scene) 
{
    draw_crosshairs();
	G_StartFrame();
    G_RenderShadowDepth(scene); 
    G_RenderToHDRColorBuffer(scene); 
    G_RenderOverlay(); 
    R_DrawLines();    
    G_RenderFinalFrame();
}

void G_RenderOverlay() 
{ 
    static char text[256];
    glm::vec3 bottomLeft = glm::vec3(-1.0f, -1.0f, 0.0f);
    glm::vec3 dimensions = glm::vec3(2.0f, 0.3f, 0.0f);
    R_DrawColoredRect(bottomLeft, dimensions, glm::vec3(0.0f, 0.0f, 0.0f)); 
    sprintf(text, "(%g, %g, %g)", main_player.Position.x, main_player.Position.y, main_player.Position.z);
    R_DrawText(text, 0, 0, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), GameFont::Anton);
}

void G_RenderEditorGrid(editor_render_context & rendercontext) 
{ 
    float width = gContext.windowWidth;
    float height = gContext.windowHeight; 

    glViewport(0, 0, width , height );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    
    BIND_SHADER(editorGrid);
    glm::vec3 & position = rendercontext.cameraPosition;
    glm::mat4 & clipToWorld = rendercontext.clipToWorld;

    glUniform2f(glGetUniformLocation(editorGrid.shader.programID, "cameraPosition"),position.x, position.z);

    glUniformMatrix4fv(
        glGetUniformLocation(editorGrid.shader.programID, "clipToWorld"), 
        1, GL_FALSE, &clipToWorld[0][0]);

    R_RenderFullScreenQuad();
    unbind_shader();
}

void G_RenderLevelEditor(editor_render_context & renderContext) 
{ 
    G_RenderEditorGrid(renderContext);
    R_DrawLines();  
}