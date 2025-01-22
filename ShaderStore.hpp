#ifndef _SHADER_STORE_HPP
#define _SHADER_STORE_HPP
#include "Graphics/GLShader.hpp"

#define SHADER_ENTRY(value, code, vert, frag, uniform_type) code = value,
enum ShaderCode 
{     
    #include "shader_entries.hpp"
};
#undef SHADER_ENTRY

// NOTE: UPDATE THIS WHENEVER YOU MODIFY THE SHADER ENTRY FILE!!!!!
#define NUM_SHADERS 10

struct shader_entry 
{ 
    ShaderCode shaderCode;    
    unsigned int uniformStructSize;
    unsigned int uniformStructOffset;
};

struct shadow_depth_ids 
{ 
    unsigned int local2LightSpace;
};

struct standard_shadow_ids 
{ 
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
};

struct editor_grid_ids 
{
    unsigned int cameraPosition;
    unsigned int clipToWorld;
};

struct hdr_blit_ids 
{ 
    unsigned int hdrBuffer;
    unsigned int exposure;
};

struct blur_ids 
{ 
    unsigned int colorBuffer;
};

struct colored_rect_ids 
{ 
    unsigned int lowerLeft;
    unsigned int dimensions;
    unsigned int color;
};

struct textured_rect_ids 
{ 
    unsigned int lowerLeft;
    unsigned int dimensions;
    unsigned int texID;    
};

struct text_render_ids
{
    unsigned int texColor;
    unsigned int text;
    unsigned int projection;
};

struct default_particle_ids
{
    unsigned int projection;
    unsigned int view;
    unsigned int mainTex;
    unsigned int noiseTex;
};

struct line_ids 
{ 
    // no parameters    
};

struct shader_data 
{ 
    unsigned int programID;
    void* uniformIDS;
};

void initialize_shader_store() ;
void release_shader_store() ;
shader_data bind_shader(ShaderCode code);
void* get_uniform_ids_internal(ShaderCode code);
unsigned int program_id(ShaderCode code);
#define get_uniform_ids(shaderCode, T) (T*) get_uniform_ids_internal(shaderCode)
#endif