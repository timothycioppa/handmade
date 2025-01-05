#include "r_main.hpp"
#include "scene_object.hpp"
#include "bsp.hpp"
#include <map>

unsigned int quadVAO;
unsigned int quadVBO;

shader_coloredRect coloredRect;
coloredrect_uniforms coloredRectUniforms;

shader_texturedRect texturedRect;
texrect_uniforms texRectUniforms;

shader_shadowed standardShadowed;
shadowed_uniforms shadowedUniforms;

shader_render_lines standardLine;
line_render_uniforms standardLineUniforms;

std::map <GameFont, FontInfo> fontmap;

static_mesh quad;

void initFSQ(); // initialize mesh for full screen quad
void initializeLineBuffers() ; // initialize mesh for line rendering

#define MAX_LINES 100

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
    COMPILE_SHADER("Shaders/coloredRect.vert", "Shaders/coloredRect.frag", coloredRect)
    COMPILE_SHADER("Shaders/texturedRect.vert", "Shaders/texturedRect.frag", texturedRect)
    COMPILE_SHADER("Shaders/standardShadow.vert", "Shaders/standardShadow.frag", standardShadowed)
    COMPILE_SHADER("Shaders/lineRender.vert", "Shaders/lineRender.frag", standardLine)
    
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
    RELEASE_SHADER(standardLine)
    RELEASE_SHADER(coloredRect)
    RELEASE_SHADER(texturedRect)
    RELEASE_SHADER(standardShadowed)

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

    BIND_SHADER(standardLine);
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
    BIND_SHADER(coloredRect);

    coloredRectUniforms.color = c;
    coloredRectUniforms.dimensions = d;
    coloredRectUniforms.lowerLeft = ll;    
    set_uniforms(coloredRect, coloredRectUniforms);

    R_RenderFullScreenQuad();
    unbind_shader();
}

void R_DrawTexturedRect (glm::vec3 ll, glm::vec3 d, texture_info* texture) 
{ 
    BIND_SHADER(texturedRect);
    texRectUniforms.dimensions = d;
    texRectUniforms.lowerLeft = ll;
    texRectUniforms.mainTex = texture->textureID; 
    set_uniforms(texturedRect, texRectUniforms);    
    R_RenderFullScreenQuad();
    unbind_shader();
}

void SHADOW_RENDER(node_render_data & renderData, RenderContext & context, bsp_tree & tree)
{    
    if (renderData.rendered)
    {
        return;
    }


    renderData.rendered = true;
    bool highlight = IS_HIGHLIGHTED(renderData);
    float scale = highlight ? 10.0f : 1.0f;

    shadowedUniforms.diffuse =      scale * renderData.material.diffuse; 
    shadowedUniforms.specular =     scale * renderData.material.specular; 
    shadowedUniforms.shininess =    scale * renderData.material.shininess; 
    shadowedUniforms.mainTex =      renderData.material.mainTexture->textureID; 
    shadowedUniforms.model =        renderData.transform.localToWorldMatrix(); 
    shadowedUniforms.modelView =    context.v * shadowedUniforms.model; 
    shadowedUniforms.modelViewProjection = context.p * shadowedUniforms.modelView; 
  
    set_uniforms(standardShadowed, shadowedUniforms); 
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

void R_RenderMeshStandardShadowed(bsp_tree & scene,  RenderContext & context)
{
    BIND_SHADER(standardShadowed)

    shadowedUniforms.cameraForward = context.cameraForward;
    shadowedUniforms.shadowMap = context.shadowMapID;
    shadowedUniforms.view = context.v;
    shadowedUniforms.lightSpace = context.lightSpace;
    shadowedUniforms.lightPosition = context.lightPosition;
    shadowedUniforms.lightColor = context.lightColor;
    shadowedUniforms.lightStrength = context.lightPower;
    shadowedUniforms.cameraPosition = context.cameraPosition;
    shadowedUniforms.time = context.totalTime;
    shadowedUniforms.delta = context.deltaTime;
    shadowedUniforms.cosTime = context.cosTime;
    shadowedUniforms.sinTime = context.sinTime;

  //  glEnable(GL_CULL_FACE);

    char lightBuff[64];
    
    for (int i = 0; i < scene.lightCount; i++) 
    {
        light & l = scene.lights[i];

        sprintf(lightBuff, "lights[%d].position", i);
        unsigned int posID =  glGetUniformLocation(standardShadowed.shader.programID, lightBuff);
        set_float3(posID, l.Position);
        
        sprintf(lightBuff, "lights[%d].color", i);
        posID =  glGetUniformLocation(standardShadowed.shader.programID, lightBuff);
        set_float3(posID, l.Color);
        
        sprintf(lightBuff, "lights[%d].intensity", i);
        posID =  glGetUniformLocation(standardShadowed.shader.programID, lightBuff);
        set_float(posID, l.intensity);
    }

    render_shadowed_recursive(scene.root, scene, context);
    unbind_shader(); 
 //   glDisable(GL_CULL_FACE);

}


void initFSQ() 
{ 

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