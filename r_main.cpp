#include "r_main.hpp"
#include "bsp.hpp"
#include <map>

#include "player.hpp"
#include "TextureStore.hpp"
#include "ShaderStore.hpp"

unsigned int quadVAO;
unsigned int quadVBO;

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

std::map <GameFont, FontInfo> fontmap;

#define FONT_ENTRY(index, code, path) fontmap.insert({ GameFont::code, FontInfo() }); \
  InitializeFont(path, WINDOW_WIDTH, WINDOW_HEIGHT, &fontmap[GameFont::code]); \

void R_Init() 
{
    initFSQ();
    #include "font_entries.hpp"
}
#undef FONT_ENTRY


void R_Cleanup() 
{ 
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
        glUniform3f(uniformIDS.dimensions, d.x, d.y, d.z);
        glUniform3f(uniformIDS.lowerLeft, ll.x, ll.y, ll.z);
        set_texture(uniformIDS.texID, texture->textureID, 0);
        R_RenderFullScreenQuad();
    }
    unbind_shader();
}

void R_DrawMeshInstanced(static_mesh & m, unsigned int count) 
{ 
    glBindVertexArray(m.VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, m.vertexCount, count);
    glBindVertexArray(0);
}

void R_DrawMesh(static_mesh & m) 
{ 
    glBindVertexArray(m.VAO);
    glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);     
    glBindVertexArray(0);
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
    {
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
    }
    
    unbind_shader();
    glEnable(GL_DEPTH_TEST);

    /// reset queue data
    _head = 0;
    _vertexCount = 0;
    _linesToRender = 0;
}


#define TEXT_BUFF_SIZE 256
#define MAX_TEXT_COMMANDS 256
struct DrawTextCommand
{
    char buff[TEXT_BUFF_SIZE];
    glm::vec2 position;
    float size;
    glm::vec3 color;
    FontInfo *font;
};

DrawTextCommand textCommands[MAX_TEXT_COMMANDS];
unsigned int textHead = 0;
unsigned int textTail = 0;

void R_DrawText(const char* text, float x, float y, float scale, glm::vec3 color, GameFont font)
{     
    FontInfo & fontInfo = fontmap[font];
    DrawTextCommand & command = textCommands[textHead];
    memset(command.buff, 0, TEXT_BUFF_SIZE);    
    strcpy(command.buff, text);
    command.position = {x, y};
    command.size = scale;
    command.color = color;
    command.font = &fontInfo; 
    textHead = (textHead + 1) % MAX_TEXT_COMMANDS;
 }

void R_DrawAllText() 
{ 
    while (textTail != textHead)
    {
        DrawTextCommand & command = textCommands[textTail];
        DrawText(command.buff, command.position.x, command.position.y, command.size, command.color, command.font);       
        textTail = (textTail + 1) % MAX_TEXT_COMMANDS; 
    }
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