#include "ShaderStore.hpp"

#define SHADER_ENTRY(value, code, vert, frag, uniform_type) {ShaderCode::code, sizeof(uniform_type)},
shader_entry entries[NUM_SHADERS] = 
{
    #include "shader_entries.hpp"
};
#undef SHADER_ENTRY

compiled_shader compiledShaders[NUM_SHADERS];
void* ShaderUniformIdBuffer;

void init_uniforms(shadow_depth_ids *ids, compiled_shader & s)
{
    unsigned int pid = s.programID;
}

void init_uniforms(standard_shadow_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;

}

void init_uniforms(editor_grid_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;

}

void init_uniforms(hdr_blit_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;
}

void init_uniforms(blur_ids *ids, compiled_shader & s) 
{
    unsigned int pid = s.programID;
}

void init_uniforms(colored_rect_ids *ids, compiled_shader & s) 
{
    // no uniforms
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

#define GET_UNIFORM_IDS(code, uniform_type) (uniform_type *) ((unsigned char*) ShaderUniformIdBuffer + entries[int(code)].uniformStructOffset)

void initialize_shader_store() 
{
    unsigned int size = 0; 

    for (int i = 0; i < NUM_SHADERS; i++) 
    {
        entries[i].uniformStructOffset = size;
        size += entries[i].uniformStructSize;
    }

    ShaderUniformIdBuffer = malloc(size);

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
    free (ShaderUniformIdBuffer);
}

void bind_shader(ShaderCode code) 
{
    compiled_shader & shader = compiledShaders[int(code)];
    bind_shader(shader);
}