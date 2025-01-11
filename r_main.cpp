#include "r_main.hpp"
#include "scene_object.hpp"
#include "bsp.hpp"
#include <map>

#include "player.hpp"
#include "TextureStore.hpp"
#include "ShaderStore.hpp"

unsigned int quadVAO;
unsigned int quadVBO;




std::map <GameFont, FontInfo> fontmap;

static_mesh quad;

void initFSQ(); // initialize mesh for full screen quad
void initializeLineBuffers() ; // initialize mesh for line rendering

#define MAX_LINES 1000

unsigned int lineVAO;
unsigned int lineVBO;
unsigned int colorVBO;

glm::vec2 lineData[2 * MAX_LINES];
glm::vec3 colorData[2 * MAX_LINES];
bool inited = false;

int _head = 0;
int _linesToRender = 0;
int _vertexCount = 0;

void R_Init() 
{
    initialize_shader_store();

    load_mesh("Models/wall.obj", quad);

    // generate mesh for full screen quad rendering
    initFSQ();

    // generate font data
    fontmap.insert({ GameFont::Ariel, FontInfo() });
    FontInfo & ariell = fontmap[GameFont::Ariel];
    InitializeFont( "C:\\Users\\josel\\code\\cpp\\handmade\\Fonts\\arial.ttf", WINDOW_WIDTH, WINDOW_HEIGHT, &ariell);

    fontmap.insert({ GameFont::Anton, FontInfo() });
    FontInfo & anton = fontmap[GameFont::Anton];
    InitializeFont( "C:\\Users\\josel\\code\\cpp\\handmade\\Fonts\\Anton.ttf", WINDOW_WIDTH, WINDOW_HEIGHT, &anton);
}


void R_Cleanup() 
{ 
    release_shader_store();
    release_mesh(quad);

    // delete FSQ gpu data
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);

    // delete line gpu data
    if (inited) 
    {
        glDeleteVertexArrays(1, &lineVAO);
        glDeleteBuffers(1, &lineVBO);
        glDeleteBuffers(1, &colorVBO);
    }
}

void R_DrawLine(glm::vec2 start, glm::vec2 end, glm::vec3 color) 
{
    if (_linesToRender == MAX_LINES) 
    {
        printf("max line rendering limit reached\n");
        return;
    }

    lineData[_head] = start;
    lineData[_head + 1] = end;

    colorData[_head] = color;
    colorData[_head + 1] = color;
    
    _linesToRender = _linesToRender + 1;
    _vertexCount = _vertexCount + 2;
    _head = _head + 2;
}


void R_DrawLines()
{
    if (!inited) 
    {     
        inited = true;
        initializeLineBuffers();
    }

    if (_linesToRender < 1) 
    {
        return;
    }

    bind_shader(ShaderCode::LINE);    
    
    glDisable(GL_DEPTH_TEST);
     
    glBindVertexArray(lineVAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, _vertexCount * sizeof(glm::vec2), &lineData[0]);     
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, _vertexCount * sizeof(glm::vec3), &colorData[0]);         
        glDrawArrays(GL_LINES, 0, _vertexCount);
    }

    glBindVertexArray(0);
    
    unbind_shader();
    glEnable(GL_DEPTH_TEST);

    /// reset queue data
    _head = 0;
    _vertexCount = 0;
    _linesToRender = 0;
}


void R_DrawText(std::string text, float x, float y, float scale, glm::vec3 color, GameFont font)
{ 
    FontInfo & fontInfo = fontmap[font];
    DrawText(text, x, y, scale, color, &fontInfo);    
}

void R_RenderFullScreenQuad() 
{
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void R_DrawColoredRect(glm::vec3 ll, glm::vec3 d, glm::vec3 c) 
{ 
    shader_data sData = bind_shader(ShaderCode::COLORED_RECT);
    colored_rect_ids& uniformIDS = *((colored_rect_ids*) sData.uniformIDS);
    
    {
        set_float3(uniformIDS.dimensions, d);
        set_float3(uniformIDS.lowerLeft, ll);
        set_float3(uniformIDS.color, c);
        R_RenderFullScreenQuad();
    }

    unbind_shader();
}

void R_DrawTexturedRect (glm::vec3 ll, glm::vec3 d, texture_info* texture) 
{ 
    shader_data sData = bind_shader(ShaderCode::TEXTURED_RECT);
    textured_rect_ids& uniformIDS = *((textured_rect_ids*) sData.uniformIDS);
    
    {
        set_float3(uniformIDS.dimensions, d);
        set_float3(uniformIDS.lowerLeft, ll);
        set_texture(uniformIDS.texID, texture->textureID, 0);
        R_RenderFullScreenQuad();
    }

    unbind_shader();
}



void SHADOW_RENDER(node_render_data & renderData, RenderContext & context, bsp_tree & tree)
{    
    if (renderData.rendered)
    {
        return;
    }

    standard_shadow_ids & uniformIDS = *get_uniform_ids(ShaderCode::STANDARD_SHADOWED, standard_shadow_ids);


    renderData.rendered = true;
    bool highlight = IS_HIGHLIGHTED(renderData);
    float scale = highlight ? 100.0f : 1.0f;

    glm::mat4 model = renderData.transform.localToWorldMatrix();
    glm::mat4 modelView = context.v * model;
    glm::mat4 modelViewProj = context.p * modelView;

    // material uniforms
    set_texture(uniformIDS.mainTexID, renderData.material.mainTexture->textureID, 0);
    set_float3(uniformIDS.diffuseID,scale *  renderData.material.diffuse);
    set_float3(uniformIDS.specularID, scale * renderData.material.specular);
    set_float(uniformIDS.shininessID, scale * renderData.material.shininess);

    // matrix uniforms
    set_mat4(uniformIDS.modelID, model);
    set_mat4(uniformIDS.modelViewID, modelView);
    set_mat4(uniformIDS.modelViewProjID, modelViewProj);

    render_mesh(quad);  
}

void doRender(const renderable_index & indices, bsp_tree & tree, RenderContext & context) 
{ 
    if (indices.renderableIndex0 > -1) 
    {
        node_render_data & renderData = tree.renderables[indices.renderableIndex0];         
        SHADOW_RENDER(renderData, context, tree);
    }

    if (indices.renderableIndex1 > -1) 
    {  
        node_render_data & renderData = tree.renderables[indices.renderableIndex1];   
        SHADOW_RENDER(renderData, context, tree);
    }    
}

void renderAdjacentSectors(const wall_segment & segment, bsp_tree & tree, RenderContext & context) 
{         
    int frontSector = segment.frontSectorID;

    if (segment.frontSectorID > -1)
    {
        sector & s = tree.sectors[frontSector];
        renderable_index & indices = s.renderIndices;
        doRender(indices, tree, context);
    }

    int backSector = segment.backSectorID;

    if (segment.backSectorID > -1)
    {
        sector & s = tree.sectors[backSector];
        renderable_index & indices = s.renderIndices;
        doRender(indices, tree, context);
    }

}

void render_shadowed_recursive(bsp_node * node, bsp_tree & tree, RenderContext & context) 
{
    if (node->front != nullptr) 
    {
        render_shadowed_recursive(node->front, tree, context);
    } 
    else
    {
        renderAdjacentSectors(tree.segments[node->segmentIndex], tree, context);
    }

    if (node->back != nullptr) 
    {
        render_shadowed_recursive(node->back, tree, context);
    }
    else
    {
        renderAdjacentSectors(tree.segments[node->segmentIndex], tree, context);
    }

    renderable_index & indices = tree.segments[node->segmentIndex].renderIndices;
    doRender(indices, tree, context);    

}


void R_RenderMeshShadowed(static_mesh & mesh, glm::mat4 & model, bsp_tree & scene, RenderContext & context, int texID) { 
   
    shader_data shaderData = bind_shader(ShaderCode::STANDARD_SHADOWED);
    standard_shadow_ids *uniformIDS = (standard_shadow_ids*) shaderData.uniformIDS;
    
    char lightBuff[64];
    
    for (int i = 0; i < scene.lightCount; i++) 
    {
        light & l = scene.lights[i];

        sprintf(lightBuff, "lights[%d].position", i);
        unsigned int posID =  glGetUniformLocation(shaderData.programID, lightBuff);
        set_float3(posID, l.Position);
        
        sprintf(lightBuff, "lights[%d].color", i);
        posID =  glGetUniformLocation(shaderData.programID, lightBuff);
        set_float3(posID, l.Color);
        
        sprintf(lightBuff, "lights[%d].intensity", i);
        posID =  glGetUniformLocation(shaderData.programID, lightBuff);
        set_float(posID, l.intensity);
    }

    glm::mat4 modelView = main_player.camData.view * model;
    glm::mat4 modelViewProj = main_player.camData.projection * modelView;

    set_texture(uniformIDS->shadowMapID,  context.shadowMapID, 1);
    set_mat4(uniformIDS->viewID, context.v);
    set_mat4(uniformIDS->lightSpaceID, context.lightSpace);
    set_float3(uniformIDS->cameraPositionID, context.cameraPosition);
    set_float3(uniformIDS->cameraForwardID, context.cameraForward);
    set_float3(uniformIDS->lightPosID, context.lightPosition);
    set_float3(uniformIDS->lightColorID, context.lightColor);
    set_float(uniformIDS->lightStrengthID, context.lightPower);
    set_float(uniformIDS->appTimeID, context.totalTime);
    set_float(uniformIDS->deltaTimeID,context.deltaTime);
    set_float(uniformIDS->cosTimeID, context.cosTime);
    set_float(uniformIDS->sinTimeID, context.sinTime);
    set_texture(uniformIDS->mainTexID, texID, 0);
    set_float3(uniformIDS->diffuseID, {1,1,1});
    set_float3(uniformIDS->specularID, {1,1,1});
    set_float(uniformIDS->shininessID, 1.0f);
    set_mat4(uniformIDS->modelID, model);
    set_mat4(uniformIDS->modelViewID, modelView);
    set_mat4(uniformIDS->modelViewProjID, modelViewProj);

    render_mesh(mesh);
    unbind_shader(); 
}


void R_RenderMeshShadowed_OLD(static_mesh & mesh, glm::mat4 & model, bsp_tree & scene, RenderContext & context, int texID) { 
   
    shader_data sData = bind_shader(ShaderCode::STANDARD_SHADOWED);
    unsigned int programID = sData.programID;
    standard_shadow_ids & uniformIDS = *((standard_shadow_ids *) sData.uniformIDS);

    char lightBuff[64];
    
    for (int i = 0; i < scene.lightCount; i++) 
    {
        light & l = scene.lights[i];

        sprintf(lightBuff, "lights[%d].position", i);
        unsigned int posID =  glGetUniformLocation(programID, lightBuff);
        set_float3(posID, l.Position);
        
        sprintf(lightBuff, "lights[%d].color", i);
        posID =  glGetUniformLocation(programID, lightBuff);
        set_float3(posID, l.Color);
        
        sprintf(lightBuff, "lights[%d].intensity", i);
        posID =  glGetUniformLocation(programID, lightBuff);
        set_float(posID, l.intensity);
    }
    


    glm::mat4 modelView = main_player.camData.view * model;
    glm::mat4 modelViewProj = main_player.camData.projection * modelView;


    set_texture(uniformIDS.shadowMapID,  context.shadowMapID, 1);
    set_mat4(uniformIDS.viewID, context.v);
    set_mat4(uniformIDS.lightSpaceID, context.lightSpace);
    set_float3(uniformIDS.cameraPositionID, context.cameraPosition);
    set_float3(uniformIDS.cameraForwardID, context.cameraForward);
    set_float3(uniformIDS.lightPosID, context.lightPosition);
    set_float3(uniformIDS.lightColorID, context.lightColor);
    set_float(uniformIDS.lightStrengthID, context.lightPower);
    set_float(uniformIDS.appTimeID, context.totalTime);
    set_float(uniformIDS.deltaTimeID,context.deltaTime);
    set_float(uniformIDS.cosTimeID, context.cosTime);
    set_float(uniformIDS.sinTimeID, context.sinTime);
    set_texture(uniformIDS.mainTexID, texID, 0);
    set_float3(uniformIDS.diffuseID, {1,1,1});
    set_float3(uniformIDS.specularID, {1,1,1});
    set_float(uniformIDS.shininessID, 1.0f);
    set_mat4(uniformIDS.modelID, model);
    set_mat4(uniformIDS.modelViewID, modelView);
    set_mat4(uniformIDS.modelViewProjID, modelViewProj);

    render_mesh(mesh);
    unbind_shader(); 

}

void R_RenderMeshStandardShadowed(bsp_tree & scene,  RenderContext & context)
{
    shader_data sData = bind_shader(ShaderCode::STANDARD_SHADOWED);
    unsigned int programID = sData.programID;
    standard_shadow_ids & uniformIDS = *((standard_shadow_ids *) sData.uniformIDS);

    char lightBuff[64];
    
    for (int i = 0; i < scene.lightCount; i++) 
    {
        light & l = scene.lights[i];

        sprintf(lightBuff, "lights[%d].position", i);
        unsigned int posID =  glGetUniformLocation(programID, lightBuff);
        set_float3(posID, l.Position);
        
        sprintf(lightBuff, "lights[%d].color", i);
        posID =  glGetUniformLocation(programID, lightBuff);
        set_float3(posID, l.Color);
        
        sprintf(lightBuff, "lights[%d].intensity", i);
        posID =  glGetUniformLocation(programID, lightBuff);
        set_float(posID, l.intensity);
    }

    // common uniforms for all objects rendered
    set_texture(uniformIDS.shadowMapID,  context.shadowMapID, 1);
    set_mat4(uniformIDS.viewID, context.v);
    set_mat4(uniformIDS.lightSpaceID, context.lightSpace);
    set_float3(uniformIDS.cameraPositionID, context.cameraPosition);
    set_float3(uniformIDS.cameraForwardID, context.cameraForward);
    set_float3(uniformIDS.lightPosID, context.lightPosition);
    set_float3(uniformIDS.lightColorID, context.lightColor);
    set_float(uniformIDS.lightStrengthID, context.lightPower);
    set_float(uniformIDS.appTimeID, context.totalTime);
    set_float(uniformIDS.deltaTimeID,context.deltaTime);
    set_float(uniformIDS.cosTimeID, context.cosTime);
    set_float(uniformIDS.sinTimeID, context.sinTime);

    glEnable(GL_CULL_FACE);
    render_shadowed_recursive(scene.root, scene, context);
    unbind_shader(); 
    glDisable(GL_CULL_FACE);
}


void initFSQ() 
{ 
    // the vertices are already in clip space, no vertex processing needs to be done in the vertex shader
    float quadVertices[] = 
    {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void initializeLineBuffers() 
{ 
    _linesToRender = 0;
    _vertexCount = 0;
    _head = 0;

    glGenVertexArrays(1, &lineVAO);

    glBindVertexArray(lineVAO);
    {
        glGenBuffers(1, &lineVBO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);       
        glBufferData(GL_ARRAY_BUFFER, MAX_LINES * 2 * sizeof(glm::vec2), &lineData[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);       
        glEnableVertexAttribArray(0);
        
        glGenBuffers(1, &colorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);       
        glBufferData(GL_ARRAY_BUFFER, MAX_LINES * 2 * sizeof(glm::vec3), &colorData[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);       
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }    

    glBindVertexArray(0);
}