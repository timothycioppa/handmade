#include "r_main.hpp"
#include "SceneObject.hpp"
#include <map>

unsigned int quadVAO;
unsigned int quadVBO;

shader_coloredRect coloredRect;
coloredrect_uniforms coloredRectUniforms;
shader_texturedRect texturedRect;
texrect_uniforms texRectUniforms;
shader_shadowed standardShadowed;
shadowed_uniforms shadowedUniforms;
std::map <GameFont, FontInfo> fontmap;

void R_Init() 
{
    COMPILE_SHADER("Shaders/coloredRect.vert", "Shaders/coloredRect.frag", coloredRect)
    COMPILE_SHADER("Shaders/texturedRect.vert", "Shaders/texturedRect.frag", texturedRect)
    COMPILE_SHADER("Shaders/standardShadow.vert", "Shaders/standardShadow.frag", standardShadowed)
    

    fontmap.insert({ GameFont::Ariel, FontInfo() });
    FontInfo & ariell = fontmap[GameFont::Ariel];
    InitializeFont( "C:\\Users\\josel\\code\\cpp\\handmade\\Fonts\\arial.ttf", WINDOW_WIDTH, WINDOW_HEIGHT, &ariell);

    fontmap.insert({ GameFont::Anton, FontInfo() });
    FontInfo & anton = fontmap[GameFont::Anton];
    InitializeFont( "C:\\Users\\josel\\code\\cpp\\handmade\\Fonts\\Anton.ttf", WINDOW_WIDTH, WINDOW_HEIGHT, &anton);

}


void R_Cleanup() 
{ 
    RELEASE_SHADER(coloredRect)
    RELEASE_SHADER(texturedRect)
    RELEASE_SHADER(standardShadowed)
}

void R_DrawText(std::string text, float x, float y, float scale, glm::vec3 color, GameFont font)
{ 
    FontInfo & fontInfo = fontmap[font];
    DrawText(text, x, y, scale, color, &fontInfo);
}

void R_RenderFullScreenQuad() 
{ 
    if (quadVAO == 0)
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

void R_RenderMeshStandardShadowed(SceneObject ** objects, int num,  RenderContext & context)
{
    BIND_SHADER(standardShadowed)

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

    glEnable(GL_CULL_FACE);
    
    for (int i = 0; i < num; i++) 
    { 
        SceneObject * so = objects[i];
        shadowedUniforms.diffuse = so->material.diffuse;
        shadowedUniforms.specular = so->material.specular;
        shadowedUniforms.shininess = so->material.shininess;
        shadowedUniforms.mainTex = so->material.mainTexture->textureID;
        shadowedUniforms.model = so->transform.localToWorldMatrix();
        shadowedUniforms.modelView = context.v * shadowedUniforms.model;
        shadowedUniforms.modelViewProjection = context.p * shadowedUniforms.modelView;

        set_uniforms(standardShadowed, shadowedUniforms);
        render_mesh(so->mesh);
    }

    unbind_shader(); 
    glDisable(GL_CULL_FACE);
}
