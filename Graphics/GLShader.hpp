#ifndef _JC_SHADER_H
#define _JC_SHADER_H

#include "../utils.hpp"
#include "../platform_common.hpp"
#include <vector>

#define COMPILE_SHADER(v, f, s) do { \
    load_shader((v), (f), (s).shader); \
    shader_init_uniforms(s); \
} while (0); \

#define BIND_SHADER(s) do { \
    bind_shader((s).shader); \
} while (0); \

#define RELEASE_SHADER(s) do { \
    release_shader((s).shader); \
} while (0); \

struct compiled_shader
{ 
    unsigned int programID;
    unsigned int vertexID;
    unsigned int fragmentID;
};
struct depth_Uniforms 
{ 
    glm::mat4 local2LightSpace;
};

struct coloredrect_uniforms
{
    glm::vec3 color;
    glm::vec3 dimensions;
    glm::vec3 lowerLeft;
};
struct texrect_uniforms
{
    unsigned int mainTex;
    glm::vec3 dimensions;
    glm::vec3 lowerLeft;
};

struct line_render_uniforms 
{ 
    glm::vec3 color;
};

struct screen_blit_uniforms { 
    unsigned int screenTexture;
};

struct hdr_uniforms 
{ 
    unsigned int hdrBuffer;
    unsigned int hdr;
    float exposure;
};

struct shader_render_lines { 
    compiled_shader shader;
    struct 
    {
   //     unsigned int color;
    } uniformIDS;
};

struct shader_hdr_blit
{
    compiled_shader shader;
    struct 
    { 
        unsigned int hdrBuffer;
        unsigned int hdr;
        unsigned int exposure;
    } uniformIDS;
};

struct text_render_uniforms 
{ 
    glm::vec3 textColor;
    unsigned int text;
    glm::mat4 projection;
};

struct shader_shadowed
{
    compiled_shader shader;
    struct { 
        unsigned int shadowMapID;
        unsigned int mainTexID;
        unsigned int diffuseID;
        unsigned int specularID;
        unsigned int shininessID;
        unsigned int modelID;
        unsigned int viewID;
        unsigned int modelViewID;
        unsigned int modelViewProjID;
        unsigned int lightSpaceID;
        unsigned int lightPosID;
        unsigned int lightColorID;
        unsigned int lightStrengthID;
        unsigned int cameraPositionID;
        unsigned int cameraForwardID;
        unsigned int appTimeID;
        unsigned int deltaTimeID;
        unsigned int cosTimeID;
        unsigned int sinTimeID;
    } uniformIDS;
};

struct shader_texturedRect 
{
    compiled_shader shader;
    struct { 
        unsigned int lowerLeft;
        unsigned int dimensions;
        unsigned int texID;
    } uniformsIDS;
};

struct shader_coloredRect 
{
    compiled_shader shader;
    struct { 
        unsigned int lowerLeft;
        unsigned int dimensions;
        unsigned int color;
    } uniformsIDS;
};
struct shader_depthPass 
{ 
    compiled_shader shader;
    struct { 
        unsigned int local2LightSpace;
    } uniformsIDS;

};


struct shader_blur 
{ 
    compiled_shader shader;
    struct 
    { 
       unsigned int colorBuffer;
    } uniformsIDS;
};


struct shader_grid 
{ 
    compiled_shader shader;
    struct 
    { 
       unsigned int cameraPosition;
       unsigned int clipToWorld;
    } uniformsIDS;
};

struct shader_fullscreenBlit { 
    compiled_shader shader;
    struct { 
        unsigned int screenTexture;
    } uniformIDS;
};
struct shader_text_rendering { 
    compiled_shader shader;
    struct { 
        unsigned int texColor;
        unsigned int text;
        unsigned int projection;
    } uniformIDS;
};
void shader_init_uniforms(shader_blur & shader);

void shader_init_uniforms(shader_grid & shader);
void shader_init_uniforms(shader_fullscreenBlit & shader);
void shader_init_uniforms(shader_shadowed & shader) ;
void shader_init_uniforms(shader_texturedRect & shader) ;
void shader_init_uniforms(shader_coloredRect & shader) ;
void shader_init_uniforms(shader_depthPass & shader) ;
void shader_init_uniforms(shader_text_rendering & shader);
void shader_init_uniforms(shader_hdr_blit & shader);

void shader_init_uniforms(shader_render_lines shader);
void set_uniforms(shader_render_lines & shader, line_render_uniforms & uniforms);

void set_uniforms(shader_coloredRect & shader, coloredrect_uniforms & uniforms);
void set_uniforms(shader_texturedRect & shader, texrect_uniforms & uniforms);
void set_uniforms(shader_depthPass & shader, depth_Uniforms & uniforms);
void set_uniforms(shader_fullscreenBlit & shader, screen_blit_uniforms & uniforms);
void set_uniforms(shader_text_rendering & shader, text_render_uniforms & uniforms);
void set_uniforms(shader_hdr_blit & shader, hdr_uniforms & uniforms);


void set_int(GLuint id, const int value) ;
void set_mat4(GLuint id, const glm::mat4 & matrix) ;
void set_float2(GLuint id, glm::vec2 value)  ;
void set_float3(GLuint id, glm::vec3 value);
void set_float4(GLuint id, glm::vec4 value) ;
void set_float(GLuint id, GLfloat value);
void set_texture(GLuint uniformID, GLuint textureID, GLuint texUnit);


bool load_shader(const char* vs, const char* fs, compiled_shader & result) ;
void bind_shader(compiled_shader & shader)  ;
void unbind_shader()  ;
void release_shader(compiled_shader & shader) ;

#endif