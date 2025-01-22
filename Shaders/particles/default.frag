#version 330 core

in float age;
in float lifetime;
in float normalizedAge;
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D mainTex;
uniform sampler2D noiseTex;

vec4 lerp(vec4 a, vec4 b, float t) 
{ 
    return a + t * (b - a);
}

vec3 lerp(vec3 a, vec3 b, float t) 
{ 
    return a + t * (b - a);
}

vec2 lerp(vec2 a, vec2 b, float t) 
{ 
    return a + t * (b - a);
}

void main()
{             

    // sprite sheet calculation
    int spriteIndex =  int(16.0f * normalizedAge);    
    int spriteX = spriteIndex % 4;
    int spriteY = spriteIndex / 4;
    vec2 uv = vec2(0.25f * float(spriteX), 0.25f * float(spriteY)) + 0.25f * TexCoords;

    vec4 texColor =  texture(mainTex, uv);
 
    float falloff = clamp(1.0f - normalizedAge, 0.0f, 1.0f);
    falloff = pow(falloff, 2.0f);

    vec3 col = lerp(vec3(5.0f), vec3(1.0f), falloff);
    vec4 color = texColor * vec4(col, 1.0f);
    color.a =  falloff * texColor.r;
    FragColor = color;

}