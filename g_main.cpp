#include "g_main.hpp"
#include "player.hpp"
#include "game_context.hpp"
#include "rect.hpp"
#include "r_main.hpp"

#include "math_utils.hpp"

float lightStrength = 0.5f;

shader_depthPass depthPass;
depth_Uniforms depthUniforms;
shader_fullscreenBlit fullScreenBlit;
screen_blit_uniforms blitUniforms;
shader_hdr_blit fullScreenHDRBlit;
hdr_uniforms hdrBlitUniforms;
FrameBufferInfo depthMapInfo;
FrameBufferInfo colorBufferInfo;
LightData lightData;
TextureStore gTextureRepository;
FrameBufferInfo hdrColorBufferInfo;
bool useHDR = true;
float exposure = 1.0f;

void GenerateHDRColorBuffer() 
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
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    hdrColorBufferInfo.fbo = hdrFBO;
    hdrColorBufferInfo.texture = colorBuffer;
}

void GenerateColorBuffer() 
{
    FrameBufferInfo * info = &colorBufferInfo;

	glGenFramebuffers(1, &(info->fbo));
	glBindFramebuffer(GL_FRAMEBUFFER, (info->fbo));
	glGenTextures(1, &(info->texture));
	glBindTexture(GL_TEXTURE_2D, info->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, info->texture, 0);  


	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void GenerateShadowDepthBuffer() 
{ 
    glGenFramebuffers(1, &(depthMapInfo.fbo));    
    glGenTextures(1, &(depthMapInfo.texture));
    glBindTexture(GL_TEXTURE_2D, depthMapInfo.texture);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapInfo.fbo);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapInfo.texture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void GenerateLightData() 
{
    lightData.near = 0.01f;
    lightData.far = 50.0f;
    lightData.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, lightData.near, lightData.far);
	lightData.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightData.lightSpaceMatrix = (lightData.projection) * (lightData.view);
}

void G_Init() 
{
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    COMPILE_SHADER("Shaders/depth.vert", "Shaders/depth.frag", depthPass)
    COMPILE_SHADER("Shaders/blit.vert", "Shaders/blit.frag", fullScreenBlit);
    COMPILE_SHADER("Shaders/hdr.vert", "Shaders/hdr.frag", fullScreenHDRBlit);

    GenerateHDRColorBuffer();
    GenerateShadowDepthBuffer();
    GenerateLightData();
}


void G_RenderShadowDepth(scene_data & scene) 
{ 
    BIND_SHADER(depthPass)   
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapInfo.fbo);
    glClear(GL_DEPTH_BUFFER_BIT);	                
    depthUniforms.lightSpace = lightData.lightSpaceMatrix;

    for (SceneObject & so : scene.sceneObjects)
    {      
        depthUniforms.model = so.transform.localToWorldMatrix();
        set_uniforms(depthPass, depthUniforms);
        render_mesh(so.mesh);
    }  
   
    unbind_shader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DEBUG_RenderScene(scene_data & scene) 
{ 
    static RenderContext context;

    context.shadowMapID = depthMapInfo.texture;
    context.cameraPosition = main_player.Position;
    context.lightSpace = lightData.lightSpaceMatrix;
    context.lightPosition = { 0.0f, 0.0f, 0.0f};
    context.lightColor = {50.0f, 50.0f, 50.0f};
    context.lightPower = 200.0f;
    context.totalTime = gContext.applicationTime;
    context.deltaTime = gContext.deltaTime;
    context.sinTime = gContext.sinTime;
    context.cosTime = gContext.cosTime;
    context.v = main_player.ViewMatrix;
    context.p = main_player.ProjectionMatrix;

    R_RenderMeshStandardShadowed(
        scene,
        context
    ); 
}

void G_StartFrame() 
{ 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void G_RenderColorBuffer(bool hdr, scene_data & scene) 
{ 
    glBindFramebuffer(GL_FRAMEBUFFER, hdrColorBufferInfo.fbo);  
    glClearColor(lightStrength * 0.4f, lightStrength * 0.6f, lightStrength * 0.3f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DEBUG_RenderScene(scene);
}

void G_RenderFinalFrame(bool hdr) 
{
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    BIND_SHADER(fullScreenHDRBlit);
    hdrBlitUniforms.hdrBuffer = hdrColorBufferInfo.texture;
    hdrBlitUniforms.hdr = true;
    hdrBlitUniforms.exposure = 1.0;
    set_uniforms(fullScreenHDRBlit, hdrBlitUniforms);
    R_RenderFullScreenQuad();
    unbind_shader();
}


void G_Cleanup() 
{ 
	for (auto & [key, value] : gTextureRepository.TextureMap ) 
	{ 
		release_texture(value);
	}
}

void G_RenderSceneShadowedFull(scene_data & scene) 
{ 
	G_StartFrame();
    G_RenderShadowDepth(scene);	
    G_RenderColorBuffer(useHDR, scene);
    G_RenderOverlay();
    G_RenderFinalFrame(useHDR);
}

void G_RenderOverlay() 
{ 
    glm::vec3 bottomLeft = glm::vec3(-1.0f, -1.0f, 0.0f);
    glm::vec3 dimensions = glm::vec3(2.0f, 0.3f, 0.0f);
    R_DrawColoredRect(bottomLeft, dimensions, glm::vec3(0.0f, 0.0f, 0.0f)); 

    static char text[256];
    sprintf(text, "(%g, %g, %g)", main_player.Position.x, main_player.Position.y, main_player.Position.z);
    R_DrawText(text, 0, 0, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), GameFont::Anton);

}