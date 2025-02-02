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

FrameBufferInfo shadowDepthMapFB;
FrameBufferInfo hdrColorBufferFB;

#define SPLIT3(p) (p).x, (p).y, (p).z
#define SPLIT2(p) (p).x, (p).y

// the main light used for 
// 1. shadow calculations
// 2. ambient lighting in the standard shader
LightData shadowLight;

// extra scene lights
#define MAX_LIGHTS 4
light lights[MAX_LIGHTS];

// used for rendering walls and ceilings (since everything is a quad)
static_mesh quadMesh;
static_mesh cubeMesh;
static_mesh sphereMesh;
static_mesh enemyMesh;
Material enemyMat;

float exposure = 1.0f;

struct lighting_uniforms
{
    bool initialized;
    unsigned int activeIDS[MAX_LIGHTS];
    unsigned int posIDS[MAX_LIGHTS];
    unsigned int colorIDS[MAX_LIGHTS];
    unsigned int intensityIDS[MAX_LIGHTS];
};

lighting_uniforms light_uniforms;

glm::vec2 vertCrosshairStart = glm::vec2(0.0f, -0.03f);
glm::vec2 vertCrosshairEnd = glm::vec2(0.0f, 0.03f);
glm::vec2 horCrosshairStart = glm::vec2(-0.03f, 0.0f);
glm::vec2 horCrosshairEnd = glm::vec2(0.03f, 0.0f);
glm::vec3 crossColor = glm::vec3(1,0,0);

RenderContext gRenderContext;

struct pipeline_settings 
{ 
    bool useBloom;
    unsigned int bloomIterations;
    float hdrExposure;
    bool shadows;
};

pipeline_settings gPipeline = 
{ 
    /* useBloom */              true,
    /* bloom Iterations */      10,
    /* hdr exposure level*/     1.0f,
    /* use shadows */           true
};

void    GenerateBloomBlendBuffers();
void    GenerateHDRColorBuffer();
void    GenerateShadowDepthBuffer();
void    InitializeShadowLight();
void    InitializeSceneLights();
void    init_light_uniforms() ;
void    draw_renderable(node_render_data & renderable, RenderContext & context, bsp_tree & tree);
void    draw_renderables(const renderable_index & indices, bsp_tree & tree, RenderContext & context) ;
void    render_adjacent_sectors(const wall_segment & segment, bsp_tree & tree, RenderContext & context) ;
void    render_scene_recursive(bsp_node * node, bsp_tree & tree, RenderContext & context) ;
void    debug_line (glm::vec3 _start, glm::vec3 _end, glm::vec3 color, camera_data & cam);
void    draw_crosshairs() ;
void    render_shadow_depth_recursive(bsp_node* node, bsp_tree & tree);


unsigned int pingpongFBO[2];
unsigned int pingpongBuffer[2];

void G_Init() 
{
    initialize_shader_store();
    init_light_uniforms();
    
    init_particles("Models/wall.obj");
   
    load_mesh("Models/wall.obj", quadMesh);
    load_mesh("Models/cube.obj", cubeMesh);
    load_mesh("Models/sphere.obj", sphereMesh);
    load_mesh("Models/suzanne.obj", enemyMesh);

    GenerateHDRColorBuffer();
    GenerateShadowDepthBuffer();
    GenerateBloomBlendBuffers();
    InitializeShadowLight();
    InitializeSceneLights();


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void G_Cleanup() 
{ 
    release_shader_store();
    release_texture_store();

    release_mesh(quadMesh);
    release_mesh(cubeMesh);
    release_mesh(sphereMesh);
}

void GenerateHDRColorBuffer() 
{ 
    unsigned int hdrFBO;
    unsigned int colorBuffer;
    unsigned int normalBuffer;
    unsigned int brightnessBuffer;
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
        glGenTextures(1, &brightnessBuffer);
        glBindTexture(GL_TEXTURE_2D, brightnessBuffer);
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
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, brightnessBuffer, 0);


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
    hdrColorBufferFB.positionTexture = brightnessBuffer;
    hdrColorBufferFB.depthTexture = rboDepth;
}

void GenerateBloomBlendBuffers() 
{
    // blur buffers used for bloom
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y, 0, GL_RGBA, GL_FLOAT, NULL );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0 );
    }
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
    for (int i = 0; i < tree.numRenderables; i++) 
    {
       if (tree.renderables[i].material.mainTexture == nullptr)
       {
            tree.renderables[i].material.mainTexture = get_texture("Textures/wood.png");
       } 
    }
}

void InitializeShadowLight() 
{
    shadowLight.position = glm::vec3(5, 5, 5);
    shadowLight.color = glm::vec3(1,1,1);
    shadowLight.intensity = 0.3f;

    shadowLight.near = 0.01f;
    shadowLight.far = 100.0f;
    float halfOrthoWidth = 10.0f;
    float halfOrthoHeight = 10.0f;

    glm::vec3 forward = { 0, -1, 0 };
    shadowLight.projection = glm::ortho(-halfOrthoWidth, halfOrthoWidth, -halfOrthoHeight, halfOrthoHeight, shadowLight.near, shadowLight.far);
	shadowLight.view = glm::lookAt(shadowLight.position, shadowLight.position + forward, glm::vec3(0.0, 1.0, 0.0));
	shadowLight.lightSpaceMatrix = (shadowLight.projection) * (shadowLight.view);
}

void InitializeSceneLights() 
{ 
    for (int i = 0; i < MAX_LIGHTS; i++) 
    { 
        light & l = lights[i];
        l.active = 0;
        l.Color = {1,1,1};
        l.intensity = 1.0f;
        l.Position = {0,0,0};
    }
}

#define LIGHT_SPACE(l, index) l.lightSpaceMatrix * tree.renderables[index].transform.localToWorld;   

void G_RenderShadowDepth(bsp_tree & scene) 
{ 
    bind_shader(ShaderCode::SHADOW_DEPTH); 
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowDepthMapFB.fbo);
    glClear(GL_DEPTH_BUFFER_BIT);	                 
    render_shadow_depth_recursive(scene.root, scene);
    unbind_shader();    

    // reset to HDR buffer for rendering
    glBindFramebuffer(GL_FRAMEBUFFER, hdrColorBufferFB.fbo);  
    glViewport(0, 0, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);
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

    // draw the segment's renderables
    wall_segment & segment = tree.segments[node->segmentIndex];
    renderable_index & indices = segment.renderIndices;
    shadow_depth_ids * ids = get_uniform_ids(ShaderCode::SHADOW_DEPTH, shadow_depth_ids);

    if (indices.renderableIndex0 > -1)
    {    
        glm::mat4 local2LightSpace = LIGHT_SPACE(shadowLight, indices.renderableIndex0);           
        glUniformMatrix4fv(ids->local2LightSpace, 1, GL_FALSE, glm::value_ptr(local2LightSpace));
        R_DrawMesh(quadMesh);
    }

    if (indices.renderableIndex1 > -1)
    {
        glm::mat4 local2LightSpace = LIGHT_SPACE(shadowLight, indices.renderableIndex1);                     
        glUniformMatrix4fv(ids->local2LightSpace, 1, GL_FALSE, glm::value_ptr(local2LightSpace));
        R_DrawMesh(quadMesh);
    }
}

void G_RenderScene(bsp_tree & scene, RenderContext & context) 
{
    shader_data sData = bind_shader(ShaderCode::STANDARD_SHADOWED);
    unsigned int programID = sData.programID;
    standard_shadow_ids & uniformIDS = *((standard_shadow_ids *) sData.uniformIDS);

    for (int i = 0; i < MAX_LIGHTS; i++) 
    {
        light & l = lights[i];
        glUniform1i(light_uniforms.activeIDS[i], l.active);

        if (l.active == 1) 
        {
            glUniform3f(light_uniforms.posIDS[i], l.Position.x, l.Position.y, l.Position.z);
            glUniform3f(light_uniforms.colorIDS[i], l.Color.x, l.Color.y, l.Color.z);
            glUniform1f(light_uniforms.intensityIDS[i], l.intensity);
        }
    }

    // common uniforms for all objects rendered
    glUniform1i(uniformIDS.useShadows, gPipeline.shadows ? 1 : 0);
    
    if (gPipeline.shadows) 
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, context.shadowMapID); 
        glUniform1i(uniformIDS.shadowMapID, 1); 
    }

    glUniformMatrix4fv(uniformIDS.viewID, 1, GL_FALSE, glm::value_ptr( context.v));
    glUniformMatrix4fv(uniformIDS.lightSpaceID, 1, GL_FALSE, glm::value_ptr( context.lightSpace));
    glUniform3f(uniformIDS.cameraPositionID, SPLIT3(context.cameraPosition));
    glUniform3f(uniformIDS.cameraForwardID, SPLIT3(context.cameraForward));
    glUniform3f(uniformIDS.lightPosID, SPLIT3(context.lightPosition));
    glUniform3f(uniformIDS.lightColorID, SPLIT3(context.lightColor));
    glUniform1f(uniformIDS.lightStrengthID, context.lightPower);
    glUniform1f(uniformIDS.appTimeID, context.totalTime);
    glUniform1f(uniformIDS.deltaTimeID,context.deltaTime);
    glUniform1f(uniformIDS.cosTimeID, context.cosTime);
    glUniform1f(uniformIDS.sinTimeID, context.sinTime);

    glEnable(GL_CULL_FACE);
    render_scene_recursive(scene.root, scene, context);
    unbind_shader(); 
    glDisable(GL_CULL_FACE);
}

void G_PostProcessing() 
{ 
    bind_shader(ShaderCode::BLUR);
    blur_ids* ids  = get_uniform_ids(ShaderCode::BLUR, blur_ids);
    
    int horizontal = 1, first_iteration = true;
    for (unsigned int i = 0; i < gPipeline.bloomIterations; i++)
    {
        unsigned int texID = first_iteration ? hdrColorBufferFB.positionTexture : pingpongBuffer[(horizontal + 1) % 2];
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); 
        glUniform1i(ids->horizontal, horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID); 
        glUniform1i(ids->image, 0); 
        R_RenderFullScreenQuad();
        horizontal = (horizontal + 1) % 2;
        if (first_iteration)
        {
            first_iteration = false;
        }
    }

    unbind_shader();
}

void G_RenderMeshShadowed(static_mesh & mesh, glm::mat4 & model, Material material, bsp_tree & scene, RenderContext & context)
{  
    shader_data shaderData = bind_shader(ShaderCode::STANDARD_SHADOWED);
    standard_shadow_ids *uniformIDS = (standard_shadow_ids*) shaderData.uniformIDS;
    
    for (int i = 0; i < MAX_LIGHTS; i++) 
    {
        light & l = lights[i];
        set_int(light_uniforms.activeIDS[i], l.active);
        if (l.active == 1)
        {
            glUniform3f(light_uniforms.posIDS[i], l.Position.x, l.Position.y, l.Position.z);
            glUniform3f(light_uniforms.colorIDS[i], l.Color.x, l.Color.y, l.Color.z);
            glUniform1f(light_uniforms.intensityIDS[i], l.intensity);
        }
    }

    glm::mat4 modelView = main_player.camData.view * model;
    glm::mat4 modelViewProj = main_player.camData.projection * modelView;

    glUniform1i(uniformIDS->useShadows, gPipeline.shadows ? 1 : 0);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uniformIDS->mainTexID, 0);
    glBindTexture(GL_TEXTURE_2D,  material.mainTexture->textureID);

    if (gPipeline.shadows) 
    {
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(uniformIDS->shadowMapID, 1);
        glBindTexture(GL_TEXTURE_2D,  context.shadowMapID);
    }

    glUniform3f(uniformIDS->cameraPositionID, SPLIT3(context.cameraPosition));
    glUniform3f(uniformIDS->cameraForwardID, SPLIT3(context.cameraForward));
    glUniform3f(uniformIDS->lightPosID, SPLIT3(context.lightPosition));
    glUniform3f(uniformIDS->lightColorID, SPLIT3(context.lightColor));
    glUniform1f(uniformIDS->lightStrengthID, context.lightPower);
    glUniform1f(uniformIDS->appTimeID, context.totalTime);
    glUniform1f(uniformIDS->deltaTimeID,context.deltaTime);
    glUniform1f(uniformIDS->cosTimeID, context.cosTime);
    glUniform1f(uniformIDS->sinTimeID, context.sinTime);
    glUniform3f(uniformIDS->diffuseID, SPLIT3(material.diffuse));
    glUniform3f(uniformIDS->specularID, SPLIT3(material.specular));
    glUniform1f(uniformIDS->shininessID, material.shininess); 
    glUniformMatrix4fv(uniformIDS->viewID, 1, GL_FALSE, glm::value_ptr(context.v));
    glUniformMatrix4fv(uniformIDS->modelID, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uniformIDS->modelViewID, 1, GL_FALSE, glm::value_ptr(modelView));
    glUniformMatrix4fv(uniformIDS->modelViewProjID, 1, GL_FALSE, glm::value_ptr(modelViewProj));
    glUniformMatrix4fv(uniformIDS->lightSpaceID, 1, GL_FALSE, glm::value_ptr(context.lightSpace));

    R_DrawMesh(mesh);
    glUseProgram(0);

}

void RenderEntities(bsp_tree & scene, gameplay_context & gameContext) 
{
    for (int i = 0; i < gameContext.numEntities; i++) 
    {
        game_entity & entity = gameContext.Entities[i];
        glm::mat4 localToWorld = entity.transform.localToWorld; 
        Material & mat = entity.material;

        G_RenderMeshShadowed(enemyMesh, localToWorld, mat, scene, gRenderContext);
    }
}

float calculate_intensity(float x) 
{
    return 50.0f * exp(-5.0f * (x - 0.1f) * (x - 0.1f));
}

void RenderProjectiles(bsp_tree & scene, gameplay_context & gameplayContext) 
{    
    static glm::vec3 projectilePosition = {0,0,0};

    light & explosionLight = lights[MAX_LIGHTS - 1];
    explosionLight.active = 0;

    if (gameplayContext.explosion.alive) 
    {

        explosionLight.active = 1;
        explosionLight.intensity = calculate_intensity(gameplayContext.explosion.age);
        explosionLight.Position = projectilePosition;

        render_system(gameplayContext.explosion, main_player.camData);
    }

    for (int i = 0; i < MAX_PROJECTILES - 1; i++) 
    {
        projectile & _projectile = gameplayContext.primaryWeapon.projectiles[i];
        light & projectileLight = lights[i];
        projectileLight.active = 0;

        if (_projectile.active) 
        { 
            projectileLight.active = 1;
            projectileLight.intensity = 250.0f; 
            projectileLight.Position = _projectile.position;
            projectilePosition = _projectile.position;

            G_RenderProjectile(_projectile.position, scene);
        }         
    }
}

void G_RenderSceneShadowedFull(bsp_tree & scene, gameplay_context & gameContext) 
{      
    if (gPipeline.shadows) 
    {
        G_RenderShadowDepth(scene);
    }

    G_RenderScene(scene, gRenderContext);
    G_RenderMeshShadowed(cubeMesh, gameContext.weaponTransform.localToWorld, gameContext.weaponMaterial, scene, gRenderContext);
    RenderEntities(scene, gameContext);
    RenderProjectiles(scene, gameContext);
    draw_crosshairs();
    G_RenderOverlay(); 
    
    if (gPipeline.useBloom) 
    {
        G_PostProcessing();
    }
}

char overlayDisplayText[256];
glm::vec3 bottomLeft = glm::vec3(-1.0f, -1.0f, 0.0f);
glm::vec3 dimensions = glm::vec3(2.0f, 0.3f, 0.0f);

void G_RenderOverlay() 
{ 
    R_DrawColoredRect(bottomLeft, dimensions, glm::vec3(0.0f, 0.0f, 0.0f)); 
    sprintf(overlayDisplayText, "(%g, %g, %g)", main_player.Position.x, main_player.Position.y, main_player.Position.z);
    R_DrawText(overlayDisplayText, 0, 0, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), GameFont::Anton);
}

void G_RenderLevelEditor(editor_render_context & renderContext) 
{ 
    // background grid
    shader_data sData = bind_shader(ShaderCode::EDITOR_GRID);
    editor_grid_ids & uniformIDS = *((editor_grid_ids*) sData.uniformIDS);    
    {
        glUniform2f(uniformIDS.cameraPosition , renderContext.cameraPosition.x, renderContext.cameraPosition.z);
        glUniformMatrix4fv(uniformIDS.clipToWorld, 1,  GL_FALSE,glm::value_ptr(renderContext.clipToWorld));
        R_RenderFullScreenQuad(); 
    }
    unbind_shader(); 

    
    char buff[128];
    sprintf(buff, "[%.2g,%.2g]", renderContext.cursorWorldPosition.x, renderContext.cursorWorldPosition.z);
    R_DrawText(buff, gContext.mousePosition.x, gContext.windowHeight - gContext.mousePosition.y, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), GameFont::Ariel);
}

void G_RenderProjectile(glm::vec3 position, bsp_tree & scene) 
{ 
    glm::mat4 projectileTransform = glm::translate(glm::mat4(1.0f), position);
    
    Material mat = 
    {
         get_texture("Textures/carpet.bmp"),
         {1,1,1},
         {1,1,1},
         1.0f
    };

    G_RenderMeshShadowed(cubeMesh, projectileTransform, mat, scene, gRenderContext);
}

int renderCount = 0;

void G_RenderFrame(game_state *currentState, game_context & context) 
{ 
    // initialize per frame data to be sent into the shaders
    gRenderContext.shadowMapID = shadowDepthMapFB.texture;
    gRenderContext.cameraPosition = main_player.Position;
    gRenderContext.cameraForward = main_player.Forward;
    gRenderContext.lightSpace = shadowLight.lightSpaceMatrix;
    gRenderContext.lightPosition =shadowLight.position;
    gRenderContext.lightColor = shadowLight.color;
    gRenderContext.lightPower = shadowLight.intensity;
    gRenderContext.totalTime = gContext.applicationTime;
    gRenderContext.deltaTime = gContext.deltaTime;
    gRenderContext.sinTime = gContext.sinTime;
    gRenderContext.cosTime = gContext.cosTime;
    gRenderContext.v = main_player.camData.view;
    gRenderContext.p = main_player.camData.projection;

    glBindFramebuffer(GL_FRAMEBUFFER, hdrColorBufferFB.fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0,0, WINDOW_WIDTH_RES_X, WINDOW_HEIGHT_RES_Y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // the actual game render call 
    currentState->Render(context);  

    /* after all rendering, we blit the hdr buffer to the front buffer*/
    glViewport(0, 0, gContext.windowWidth , gContext.windowHeight );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    
    shader_data data = bind_shader(ShaderCode::HDR_BLIT);    
    hdr_blit_ids& uniformIDS = *((hdr_blit_ids*) data.uniformIDS);

    {    
        glActiveTexture(GL_TEXTURE0);        
        glUniform1i(uniformIDS.hdrBuffer, 0);
        glBindTexture(GL_TEXTURE_2D, hdrColorBufferFB.texture); 
        glUniform1i(uniformIDS.useBloom, gPipeline.useBloom ? 1 : 0);

        if (gPipeline.useBloom)
        {
            glActiveTexture(GL_TEXTURE1);        
            glUniform1i(uniformIDS.blurBuffer, 1);     
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[1]);
        }
        
        glUniform1f(uniformIDS.exposure,  gPipeline.hdrExposure);

        R_RenderFullScreenQuad();
    }

    unbind_shader();
    // note(josel): this makes lines/text drawn at screen resolution, which i don't like
    R_DrawLines();    
    
    static char rc_buff[128];
    sprintf(rc_buff, "render count: [%d]", renderCount);
    R_DrawText(rc_buff, 100, 100, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), GameFont::Ariel);
    R_DrawAllText();
    renderCount = 0;
}

void init_light_uniforms() 
{ 
    if (light_uniforms.initialized) 
    { 
        return;
    }

    light_uniforms.initialized = true;    
    char lightBuff[64];
    unsigned int programID = program_id(ShaderCode::STANDARD_SHADOWED);
    
    for (int i = 0; i < MAX_LIGHTS; i++) 
    {    
    
        sprintf(lightBuff, "lights[%d].active", i);
        light_uniforms.activeIDS[i] =  glGetUniformLocation(programID, lightBuff);

        sprintf(lightBuff, "lights[%d].position", i);
        light_uniforms.posIDS[i] =  glGetUniformLocation(programID, lightBuff);
        
        sprintf(lightBuff, "lights[%d].color", i);
        light_uniforms.colorIDS[i] =  glGetUniformLocation(programID, lightBuff);
        
        sprintf(lightBuff, "lights[%d].intensity", i);
        light_uniforms.intensityIDS[i] =  glGetUniformLocation(programID, lightBuff);         
    }
}


void draw_renderable(node_render_data & renderable, RenderContext & context, bsp_tree & tree)
{    
    if (HAS_BEEN_RENDERED(renderable)) 
    {
        return;
    }

    if (!IS_VISIBLE(renderable)) {
        return;
    }

    MARK_RENDERED(renderable);
    renderCount++;
    glm::mat4 model = renderable.transform.localToWorld;
    glm::mat4 modelView = context.v * model;
    glm::mat4 modelViewProj = context.p * modelView;

    standard_shadow_ids & uniformIDS = *get_uniform_ids(ShaderCode::STANDARD_SHADOWED, standard_shadow_ids);
   
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderable.material.mainTexture->textureID);
    glUniform1i(uniformIDS.mainTexID, 0);

    glUniform3f(uniformIDS.diffuseID,   renderable.material.diffuse.x, renderable.material.diffuse.y, renderable.material.diffuse.z);
    glUniform3f(uniformIDS.specularID,  renderable.material.specular.x, renderable.material.specular.y, renderable.material.specular.z);
    glUniform1f(uniformIDS.shininessID,  renderable.material.shininess);
    glUniformMatrix4fv(uniformIDS.modelID, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uniformIDS.modelViewID, 1, GL_FALSE, glm::value_ptr(modelView));
    glUniformMatrix4fv(uniformIDS.modelViewProjID, 1, GL_FALSE, glm::value_ptr(modelViewProj));

    R_DrawMesh(quadMesh);  
}

void draw_renderables(const renderable_index & indices, bsp_tree & tree, RenderContext & context) 
{ 
    if (indices.renderableIndex0 > -1) 
    {
        node_render_data & renderable = tree.renderables[indices.renderableIndex0];         
        draw_renderable(renderable, context, tree);
    }

    if (indices.renderableIndex1 > -1) 
    {  
        node_render_data & renderable = tree.renderables[indices.renderableIndex1];   
        draw_renderable(renderable, context, tree);
    }    
}

void render_adjacent_sectors(const wall_segment & segment, bsp_tree & tree, RenderContext & context) 
{         
    if (segment.frontSectorID > -1)
    {
        sector & frontSector = tree.sectors[segment.frontSectorID];
        renderable_index & indices = frontSector.renderIndices;
        draw_renderables(indices, tree, context);
    }

    if (segment.backSectorID > -1)
    {
        sector & backSector = tree.sectors[segment.backSectorID];
        renderable_index & indices = backSector.renderIndices;
        draw_renderables(indices, tree, context);
    }
}

void render_scene_recursive(bsp_node * node, bsp_tree & tree, RenderContext & context) 
{
    if (node->front != nullptr) 
    {       
        render_scene_recursive(node->front, tree, context);
    } 
    else
    {
        render_adjacent_sectors(tree.segments[node->segmentIndex], tree, context);
    }

    if (node->back != nullptr) 
    {
        render_scene_recursive(node->back, tree, context);
    }
    else
    {
        render_adjacent_sectors(tree.segments[node->segmentIndex], tree, context);
    }

    // render the segment
    renderable_index & indices = tree.segments[node->segmentIndex].renderIndices;
    draw_renderables(indices, tree, context);    
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

void draw_crosshairs() 
{ 
    float ar = gContext.aspectRatio;
    R_DrawLine(horCrosshairStart, horCrosshairEnd, glm::vec3(1,1,1));
    R_DrawLine(vertCrosshairStart * ar, vertCrosshairEnd * ar,  glm::vec3(1,1,1));  
}