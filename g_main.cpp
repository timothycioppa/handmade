#include "g_main.hpp"
#include "player.hpp"
#include "game_context.hpp"
#include "r_main.hpp"

#include "math_utils.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#include "include/glm/gtc/type_ptr.hpp"
#include "editor_controller.hpp"
#include "ShaderStore.hpp"

float lightStrength = 0.5f;

FrameBufferInfo shadowDepthMapFB;
FrameBufferInfo hdrColorBufferFB;
LightData lightData;
TextureStore gTextureRepository;

// used for rendering walls and ceilings (since everything is a quad)
static_mesh quadMesh;
static_mesh cubeMesh;

bool useHDR = true;
float exposure = 1.0f;

void    GenerateHDRColorBuffer();
void    GenerateShadowDepthBuffer();
void    GenerateLightData();

void G_Init() 
{
    load_mesh("Models/wall.obj", quadMesh);
    load_mesh("Models/cube.obj", cubeMesh);

    GenerateHDRColorBuffer();
    GenerateShadowDepthBuffer();
    GenerateLightData();

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void G_Cleanup() 
{ 
    release_mesh(quadMesh);

	for (auto & [key, value] : gTextureRepository.TextureMap ) 
	{ 
		release_texture(value);
	}
}

void GenerateHDRColorBuffer() 
{ 
    unsigned int hdrFBO;
    unsigned int colorBuffer;
    unsigned int normalBuffer;
    unsigned int positionBuffer;
    unsigned int rboDepth;

    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    {

        // 1. generate HDR color buffer
        glGenTextures(1, &colorBuffer);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }


        // 2. generate normal textures
        glGenTextures(1, &normalBuffer);
        glBindTexture(GL_TEXTURE_2D, normalBuffer);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }


        // 3. generate position texture
        glGenTextures(1, &positionBuffer);
        glBindTexture(GL_TEXTURE_2D, positionBuffer);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }


        // 4. generate depth texture
        glGenTextures(1, &rboDepth);
        glBindTexture(GL_TEXTURE_2D, rboDepth);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }


        // attach (1-3) as color buffers, to be written into by the standard shadow shader
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalBuffer, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, positionBuffer, 0);


        unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
        

        // attach the depth buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rboDepth, 0);
        unsigned int error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (error != GL_FRAMEBUFFER_COMPLETE) 
        {
            std::cout << "Framebuffer not complete [" << error << "]" << std::endl;
        }

    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    hdrColorBufferFB.fbo = hdrFBO;
    hdrColorBufferFB.texture = colorBuffer;
    hdrColorBufferFB.normalTexture = normalBuffer;
    hdrColorBufferFB.positionTexture = positionBuffer;
    hdrColorBufferFB.depthTexture = rboDepth;
}

void GenerateShadowDepthBuffer() 
{ 
    glGenFramebuffers(1, &(shadowDepthMapFB.fbo));    

    // generate the shadow depth texture
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
            tree.renderables[i].material.mainTexture = gTextureRepository.GetTexture("Textures/MMCellNoise.png");
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
    shadow_depth_ids * ids = get_uniform_ids(ShaderCode::SHADOW_DEPTH, shadow_depth_ids);

    if (indices.renderableIndex0 > -1)
    {
        glm::mat4 local2LightSpace = 
            tree.lights[0].lightSpace * tree.renderables[indices.renderableIndex0].transform.localToWorldMatrix();             
        glUniformMatrix4fv(ids->local2LightSpace, 1, GL_FALSE, glm::value_ptr(local2LightSpace));
        render_mesh(quadMesh);
    }

    if (indices.renderableIndex1 > -1)
    {
        glm::mat4 local2LightSpace = 
            tree.lights[0].lightSpace  * tree.renderables[indices.renderableIndex1].transform.localToWorldMatrix();                        
        glUniformMatrix4fv(ids->local2LightSpace, 1, GL_FALSE, glm::value_ptr(local2LightSpace));
        render_mesh(quadMesh);
    }
}

void G_RenderShadowDepth(bsp_tree & scene) 
{ 
    bind_shader(ShaderCode::SHADOW_DEPTH); 
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
    context.lightColor = {1.0f, 1.0f, 1.0f};
    context.lightPower = 1.0f;

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
    glViewport(0, 0, gContext.windowWidth , gContext.windowHeight );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    
    shader_data data = bind_shader(ShaderCode::HDR_BLIT);    
    hdr_blit_ids& uniformIDS = *((hdr_blit_ids*) data.uniformIDS);

    {    
        set_texture(uniformIDS.hdrBuffer,  hdrColorBufferFB.texture, 0);
        set_int(uniformIDS.hdr, true);
        set_float(uniformIDS.exposure,  1.0f);
        R_RenderFullScreenQuad();
    }

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


void G_PostProcessing() 
{ 
#ifdef DONT_RUN
    // BLURRING: 
    {
        float width = gContext.windowWidth;
        float height = gContext.windowHeight; 

        glViewport(0, 0, width , height );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        
        bind_shader(ShaderCode::BLUR);
        blur_ids* ids  = get_uniform_ids(ShaderCode::BLUR, blur_ids);
        {
            set_texture(ids->colorBuffer, hdrColorBufferFB.texture, 0);
            R_RenderFullScreenQuad();
        }

        unbind_shader();
    }
    #endif
}

void RenderGun(bsp_tree & scene) 
{ 

    static RenderContext context;
    context.shadowMapID = shadowDepthMapFB.texture;
    context.cameraPosition = main_player.Position;
    context.cameraForward = main_player.Forward;
    context.lightSpace = lightData.lightSpaceMatrix;
    
    context.lightPosition = { 0.0f, 0.0f, 0.0f};
    context.lightColor = {1.0f, 1.0f, 1.0f};
    context.lightPower = 1.0f;

    context.totalTime = gContext.applicationTime;
    context.deltaTime = gContext.deltaTime;
    context.sinTime = gContext.sinTime;
    context.cosTime = gContext.cosTime;
    context.v = main_player.camData.view;
    context.p = main_player.camData.projection;

    float amp = 0.05f;
    float speed = 8.0f;
    float offset = sin(speed * context.totalTime) * amp;   

    glm::vec3 pos = main_player.Position + main_player.Forward * 0.25f+ main_player.Right * 0.5f + glm::vec3(0.0f, offset, 0.0f);
    glm::vec3 target = main_player.Position + 20.0f * main_player.Forward;
    glm::vec3 scale = {.05, .1, .8};
    glm::mat4 m = glm::inverse(glm::lookAt(pos, target, {0,1,0})) * glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 gunTransform = glm::inverse(glm::lookAt(pos, target, {0,1,0})) * glm::scale(glm::mat4(1.0f), scale);

    {
        glBindFramebuffer(GL_FRAMEBUFFER, hdrColorBufferFB.fbo);  
        glViewport(0, 0, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);
        R_RenderMeshShadowed(cubeMesh, gunTransform, scene, context, gTextureRepository.GetTexture("Textures/wood.jpg")->textureID);
    } 
}

void G_RenderSceneShadowedFull(bsp_tree & scene) 
{
    draw_crosshairs();

	G_StartFrame();
    G_RenderShadowDepth(scene); 
    G_RenderToHDRColorBuffer(scene);

    RenderGun(scene);

    // at this point the full G buffer is accessible, though the color buffer hasn't been remapped
    // that happens in the hdr pass (G_RenderFinalFrame()) 
    G_RenderOverlay(); 
    R_DrawLines();    
    G_RenderFinalFrame();

    // post processing can happen here
    G_PostProcessing();
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
    glViewport(0, 0, gContext.windowWidth, gContext.windowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    
    shader_data sData = bind_shader(ShaderCode::EDITOR_GRID);
    editor_grid_ids & uniformIDS = *((editor_grid_ids*) sData.uniformIDS);    

    {
        glUniform2f(uniformIDS.cameraPosition , rendercontext.cameraPosition.x, rendercontext.cameraPosition.z);
        glUniformMatrix4fv(uniformIDS.clipToWorld, 1,  GL_FALSE,glm::value_ptr(rendercontext.clipToWorld));
        R_RenderFullScreenQuad(); 
    }

    unbind_shader(); 
}

void G_RenderLevelEditor(editor_render_context & renderContext) 
{ 
    G_RenderEditorGrid(renderContext);
    R_DrawLines();
    char buff[128];
    sprintf(buff, "[%.2g,%.2g]", renderContext.cursorWorldPosition.x, renderContext.cursorWorldPosition.z);
    R_DrawText(buff, gContext.mousePosition.x, gContext.windowHeight - gContext.mousePosition.y, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), GameFont::Ariel);
}

void G_RenderTitleScreen() 
{ 
    glViewport(0, 0, gContext.windowWidth, gContext.windowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.2f, .2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
}