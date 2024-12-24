#version 330 core

in vec2 TexCoords;
uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;
out vec4 FragColor;

void main()
{             
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;    
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(0.45454545454f));
    FragColor = vec4(result, 1.0);
}