#include "GLShader.hpp"


void shader_init_uniforms(shader_render_lines shader) {}

void set_uniforms(shader_render_lines & shader, line_render_uniforms & uniforms) {}

void shader_init_uniforms(shader_hdr_blit & shader) 
{
    unsigned int pID = shader.shader.programID;
    shader.uniformIDS.hdr = glGetUniformLocation(pID, "hdr");
    shader.uniformIDS.hdrBuffer = glGetUniformLocation(pID, "hdrBuffer");
    shader.uniformIDS.exposure = glGetUniformLocation(pID, "exposure");
}

void shader_init_uniforms(shader_blur & shader) 
{
    unsigned int pID = shader.shader.programID;
    shader.uniformsIDS.colorBuffer =  glGetUniformLocation(pID, "colorBuffer");
}


void shader_init_uniforms(shader_grid & shader) 
{
    unsigned int pID = shader.shader.programID;
    shader.uniformsIDS.cameraPosition =  glGetUniformLocation(pID, "cameraPosition");
    shader.uniformsIDS.clipToWorld =  glGetUniformLocation(pID, "clipToWorld");
}

void shader_init_uniforms(shader_texturedRect & shader) 
{ 
    unsigned int pID = shader.shader.programID;
    shader.uniformsIDS.texID = glGetUniformLocation(pID, "mainTex");
    shader.uniformsIDS.dimensions = glGetUniformLocation(pID, "dimensions");
    shader.uniformsIDS.lowerLeft = glGetUniformLocation(pID, "lowerLeft");
}

void shader_init_uniforms(shader_coloredRect & shader) 
{ 
    unsigned int pID = shader.shader.programID;
    shader.uniformsIDS.color = glGetUniformLocation(pID, "color");
    shader.uniformsIDS.dimensions = glGetUniformLocation(pID, "dimensions");
    shader.uniformsIDS.lowerLeft = glGetUniformLocation(pID, "lowerLeft");
}

void shader_init_uniforms(shader_depthPass & shader) 
{ 
    unsigned int pID = shader.shader.programID;
    shader.uniformsIDS.local2LightSpace = glGetUniformLocation(pID, "local2LightSpace");
}

void shader_init_uniforms(shader_fullscreenBlit & shader) {
     unsigned int pID = shader.shader.programID;
    shader.uniformIDS.screenTexture = glGetUniformLocation(pID, "screenTexture");
}


void shader_init_uniforms(shader_text_rendering & shader) { 
    unsigned int pID = shader.shader.programID;
    shader.uniformIDS.projection = glGetUniformLocation(pID, "projection");
    shader.uniformIDS.text = glGetUniformLocation(pID, "text");
    shader.uniformIDS.texColor = glGetUniformLocation(pID, "textColor");
}

void set_uniforms(shader_text_rendering & shader, text_render_uniforms & uniforms) {
    set_mat4(shader.uniformIDS.projection, uniforms.projection);
    set_float3(shader.uniformIDS.texColor, uniforms.textColor);
    set_texture(shader.uniformIDS.text, uniforms.text, 0);
}

void set_uniforms(shader_coloredRect & shader, coloredrect_uniforms & uniforms){
    set_float3(shader.uniformsIDS.dimensions, uniforms.dimensions);
    set_float3(shader.uniformsIDS.lowerLeft, uniforms.lowerLeft);
    set_float3(shader.uniformsIDS.color, uniforms.color);
}

void set_uniforms(shader_texturedRect & shader, texrect_uniforms & uniforms){
    set_float3(shader.uniformsIDS.dimensions, uniforms.dimensions);
    set_float3(shader.uniformsIDS.lowerLeft, uniforms.lowerLeft);
    set_texture(shader.uniformsIDS.texID, uniforms.mainTex, 0);
}


void set_uniforms(shader_depthPass & shader, depth_Uniforms & uniforms)
{    
    set_mat4(shader.uniformsIDS.local2LightSpace, uniforms.local2LightSpace);
}

void set_uniforms(shader_fullscreenBlit & shader, screen_blit_uniforms & uniforms)
{
    set_texture(shader.uniformIDS.screenTexture, uniforms.screenTexture, 0);
}

void set_int(GLuint id, const int value) 
{ 
    glUniform1i(id, value);
}

void set_mat4(GLuint id, const glm::mat4 & matrix) 
{ 
    glUniformMatrix4fv(id, 1, GL_FALSE, &matrix[0][0]);
}

void set_float2(GLuint id, glm::vec2 value)  
{ 
    glUniform2f(id, value.x, value.y);
}

void set_float3(GLuint id, glm::vec3 value)  
{ 
    glUniform3f(id, value.x, value.y, value.z);
}

void set_float4(GLuint id, glm::vec4 value)  
{ 
    glUniform4f(id, value.r, value.g, value.b, value.a);
}

void set_float(GLuint id, GLfloat value)
{ 
    glUniform1f(id, value);
}

void set_texture(GLuint uniformID, GLuint textureID, GLuint texUnit) 
{ 
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glUniform1i(uniformID, texUnit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

bool load_shader(const char* vs, const char* fs, compiled_shader & result) 
{ 
    GLint Result = GL_FALSE;
    int InfoLogLength;
    char* vertexShaderSource = readfile_scratch(vs);

    result.vertexID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(result.vertexID, 1, &vertexShaderSource, NULL);
    glCompileShader(result.vertexID);

    glGetShaderiv(result.vertexID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(result.vertexID, GL_INFO_LOG_LENGTH, &InfoLogLength); 

    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(result.vertexID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    result.fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
    char* fragmentShaderSource = readfile_scratch(fs);

    glShaderSource( result.fragmentID, 1, &fragmentShaderSource, NULL);
    glCompileShader( result.fragmentID);

    // Check Fragment Shader
    glGetShaderiv(result.fragmentID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(result.fragmentID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(result.fragmentID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    result.programID = glCreateProgram();
    glAttachShader( result.programID, result.vertexID);
    glAttachShader( result.programID, result.fragmentID);
    glLinkProgram( result.programID);

    // Check the program
    glGetProgramiv( result.programID, GL_LINK_STATUS, &Result);
    glGetProgramiv( result.programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog( result.programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDeleteShader(result.vertexID);
    glDeleteShader(result.fragmentID);

    return true;
}

void bind_shader(compiled_shader & shader)  { 
    glUseProgram(shader.programID);
}

void unbind_shader()  { 
    glUseProgram(0);
}

void release_shader(compiled_shader & shader)  { 
    glDeleteProgram(shader.programID);
}