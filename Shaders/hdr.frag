#version 330 core

in vec2 TexCoords;
uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;
out vec4 FragColor;

float linearize_depth(float d,float zNear,float zFar)
{
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main()
{             
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;    
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(0.45454545454f));
    FragColor = vec4(result, 1.0);
}