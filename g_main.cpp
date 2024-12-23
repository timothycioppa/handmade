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
FrameBufferInfo depthMapInfo;
FrameBufferInfo colorBufferInfo;
LightData lightData;
TextureStore gTextureRepository;

#define MAKE_WALL(o,n,w,h) { {n, o}, w, h}

room_wall walls[4] = 
{
    MAKE_WALL(glm::vec3(-5,0,0), glm::vec3(1,0,0), 10, 10),
    MAKE_WALL(glm::vec3(5,0,0), glm::vec3(-1,0,0), 10, 10),
    MAKE_WALL(glm::vec3(0,0,5), glm::vec3(0,0,-1), 10, 10),
    MAKE_WALL(glm::vec3(0,0,-5), glm::vec3(0,0,1), 10, 10)
};

#undef MAKE_WALL
Transform debugTransforms[4];
SceneObject* debugSceneobjects[4];

void init_walls() 
{
    MaterialInfo debugMatinfo;
    debugMatinfo.diffuse = {1.0f, 1.0f, 1.0f};
    debugMatinfo.shininess = 1.0f;
    debugMatinfo.specular = {1.0f, 1.0f, 1.0f};
    debugMatinfo.ShaderType = "STANDARD_SHADOW";
    debugMatinfo.MainTex = "Textures/arabesque.bmp";

    for (int i = 0; i < 4; i++) 
    { 
        room_wall & wall = walls[i];
        Transform & debugTransform = debugTransforms[i];

        debugTransform.position = wall.wall_plane.origin;
        debugTransform.scale = {wall.width, 1.0f, wall.height};
        debugTransform.axis = glm::cross(wall.wall_plane.normal, glm::vec3(0,1,0));
        debugTransform.rotation = glm::radians(90.0f);
        debugTransform.MarkDirty();
        
        debugSceneobjects[i] = new SceneObject(
            "DEBUG_SCENE_OBJECT",
            "Models/quad.obj",
            debugMatinfo,
            debugTransform
        );

        debugSceneobjects[i]->material.mainTexture = gTextureRepository.GetTexture(debugMatinfo.MainTex);
        debugSceneobjects[i]->material.diffuse =  debugMatinfo.diffuse;
        debugSceneobjects[i]->material.specular =  debugMatinfo.specular;
        debugSceneobjects[i]->material.shininess =  debugMatinfo.shininess;
    }
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

	// unsigned int rbo;
	// glGenRenderbuffers(1, &rbo);
	// glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	// glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);  
	// glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void GenerateDepthBuffer() 
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
	lightData.view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightData.lightSpaceMatrix = (lightData.projection) * (lightData.view);
}

void G_Init() 
{
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    COMPILE_SHADER("Shaders/depth.vert", "Shaders/depth.frag", depthPass)
    COMPILE_SHADER("Shaders/blit.vert", "Shaders/blit.frag", fullScreenBlit);

    GenerateColorBuffer();
    GenerateDepthBuffer();
    GenerateLightData();

    // DEBUG CODE
    init_walls();
}


void G_RenderDepth() 
{ 
    BIND_SHADER(depthPass)   
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapInfo.fbo);
    glClear(GL_DEPTH_BUFFER_BIT);	                
    
    depthUniforms.lightSpace = lightData.lightSpaceMatrix;

    for (int i = 0; i < 4; i++) 
    { 
        SceneObject * so = debugSceneobjects[i];
        depthUniforms.model = so->transform.localToWorldMatrix();
        set_uniforms(depthPass, depthUniforms);
        render_mesh(so->mesh);
    }  
   
    unbind_shader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DEBUG_RenderScene() 
{ 
    static RenderContext context;
    context.shadowMapID = depthMapInfo.texture;
    context.cameraPosition = main_player.Position;
    context.lightSpace = lightData.lightSpaceMatrix;
    context.lightPosition = { 0.0f, 0.0f, 0.0f};
    context.lightColor = {1.0f, 1.0f, 1.0f};
    context.lightPower = 50.0f;
    context.totalTime = gContext.applicationTime;
    context.deltaTime = gContext.deltaTime;
    context.sinTime = gContext.sinTime;
    context.cosTime = gContext.cosTime;
    context.v = main_player.ViewMatrix;
    context.p = main_player.ProjectionMatrix;

    R_RenderMeshStandardShadowed(
        debugSceneobjects,
        4,
        context
    );
}

void G_StartFrame() 
{ 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void G_RenderColorBuffer() 
{ 
	glBindFramebuffer(GL_FRAMEBUFFER, colorBufferInfo.fbo);
    glClearColor(lightStrength * 0.4f, lightStrength * 0.6f, lightStrength * 0.3f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DEBUG_RenderScene();
}

void G_RenderFinalFrame() 
{ 
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);
 
    BIND_SHADER(fullScreenBlit);
    blitUniforms.screenTexture = colorBufferInfo.texture;
    set_uniforms(fullScreenBlit, blitUniforms);
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

void G_RenderSceneShadowedFull() 
{ 
	G_StartFrame();
    G_RenderDepth();	
    G_RenderColorBuffer();
    G_RenderOverlay();
    G_RenderFinalFrame();
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