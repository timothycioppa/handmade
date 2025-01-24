#include "ShaderStore.hpp"

#define SHADER_ENTRY(value, code, vert, frag, uniform_type) {ShaderCode::code, sizeof(uniform_type)},
shader_entry entries[NUM_SHADERS] = 
{
    #include "shader_entries.hpp"
};
#undef SHADER_ENTRY

compiled_shader compiledShaders[NUM_SHADERS];

// NOTE(josel): we may need to expand this if we have a LOT of shader uniforms, but that would probably be crazy
unsigned char* ShaderUniformIdBuffer[4096];

void init_uniforms(shadow_depth_ids *ids, compiled_shader & s)
{
    unsigned int pid = s.programID;
    shadow_depth_ids& uniformIDS = *ids;
    uniformIDS.local2LightSpace = glGetUniformLocation(pid, "local2LightSpace");
}

void init_uniforms(standard_shadow_ids *ids, compiled_shader & s) 
{
    unsigned int pID = s.programID;
    standard_shadow_ids & uniformIDS = *ids;
    uniformIDS.useShadows = glGetUniformLocation(pID, "useShadows");
    uniformIDS.shadowMapID = glGetUniformLocation(pID, "unity_ShadowMap");
    uniformIDS.mainTexID = glGetUniformLocation(pID, "material.mainTex");
    uniformIDS.diffuseID = glGetUniformLocation(pID, "material.diffuse");
    uniformIDS.specularID = glGetUniformLocation(pID, "material.specular");
    uniformIDS.shininessID = glGetUniformLocation(pID, "material.shininess");
    uniformIDS.modelID = glGetUniformLocation(pID, "unity_M");
    uniformIDS.viewID = glGetUniformLocation(pID, "unity_V");
    uniformIDS.modelViewID = glGetUniformLocation(pID, "unity_MV");
    uniformIDS.modelViewProjID = glGetUniformLocation(pID, "unity_MVP");
    uniformIDS.lightSpaceID = glGetUniformLocation(pID, "lightSpaceMatrix");
    uniformIDS.cameraPositionID = glGetUniformLocation(pID, "unity_CameraPosition");
    uniformIDS.cameraForwardID = glGetUniformLocation(pID, "unity_CameraForward");
    uniformIDS.lightPosID = glGetUniformLocation(pID, "unity_LightPosition");
    uniformIDS.lightColorID = glGetUniformLocation(pID, "lightColor");
    uniformIDS.lightStrengthID = glGetUniformLocation(pID, "lightPower");    
    uniformIDS.appTimeID = glGetUniformLocation(pID, "time.totalTime");
    uniformIDS.deltaTimeID = glGetUniformLocation(pID, "time.deltaTime");
    uniformIDS.cosTimeID = glGetUniformLocation(pID, "time.cosTime");
    uniformIDS.sinTimeID = glGetUniformLocation(pID, "time.sinTime");
}

void init_uniforms(editor_grid_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;
    ids->cameraPosition = glGetUniformLocation(pid, "cameraPosition");
    ids->clipToWorld = glGetUniformLocation(pid, "clipToWorld");
}

void init_uniforms(hdr_blit_ids *ids, compiled_shader & s) 
{
    unsigned int pID = s.programID;
    ids->useBloom =  glGetUniformLocation(pID, "useBloom");
    ids->hdrBuffer = glGetUniformLocation(pID, "hdrBuffer");
    ids->blurBuffer = glGetUniformLocation(pID, "blurBuffer");
    ids->exposure = glGetUniformLocation(pID, "exposure");
}

void init_uniforms(blur_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;
    ids->image =  glGetUniformLocation(pid, "image");
    ids->horizontal =  glGetUniformLocation(pid, "horizontal");
}

void init_uniforms(text_render_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;
    ids->projection = glGetUniformLocation(pid, "projection");
    ids->texColor = glGetUniformLocation(pid, "textColor");
    ids->text = glGetUniformLocation(pid, "text");
}

void init_uniforms(colored_rect_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;
    ids->dimensions = glGetUniformLocation(pid, "dimensions");
    ids->lowerLeft = glGetUniformLocation(pid, "lowerLeft");
    ids->color = glGetUniformLocation(pid, "color");
}

void init_uniforms(textured_rect_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;
    ids->dimensions = glGetUniformLocation(pid, "dimensions");
    ids->lowerLeft = glGetUniformLocation(pid, "lowerLeft");
    ids->texID = glGetUniformLocation(pid, "mainTex");
}

void init_uniforms(line_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;    
}

void init_uniforms(default_particle_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;   
    ids->projection = glGetUniformLocation(pid, "projection"); 
    ids->view = glGetUniformLocation(pid, "view"); 
    ids->mainTex = glGetUniformLocation(pid, "mainTex");
    ids->noiseTex =  glGetUniformLocation(pid, "noiseTex");
}

void initialize_shader_store() 
{
    unsigned int size = 0; 

    for (int i = 0; i < NUM_SHADERS; i++) 
    {
        entries[i].uniformStructOffset = size;
        size += entries[i].uniformStructSize;
    }
    
    printf("shader uniform size: [%d]\n", size);
    #define SHADER_ENTRY(index, code, vert, frag, uniform_type) load_shader(vert, frag, compiledShaders[index]); \
    init_uniforms((uniform_type*) ((unsigned char*) ShaderUniformIdBuffer + entries[index].uniformStructOffset), compiledShaders[index]); \

        #include "shader_entries.hpp" 

    #undef SHADER_ENTRY
}

void release_shader_store() 
{ 
    for (int i = 0; i < NUM_SHADERS; i++) 
    {
        release_shader(compiledShaders[i]);
    }    
}

shader_data bind_shader(ShaderCode code) 
{
    compiled_shader & shader = compiledShaders[int(code)];
    bind_shader(shader);
    void *ids =  (void*) ((unsigned char*) ShaderUniformIdBuffer + entries[int(code)].uniformStructOffset);
    return {shader.programID, ids};
}

void* get_uniform_ids_internal(ShaderCode code) 
{ 
    return (void*) ((unsigned char*) ShaderUniformIdBuffer + entries[int(code)].uniformStructOffset);
}

unsigned int program_id(ShaderCode code) {
    return compiledShaders[int(code)].programID;
}