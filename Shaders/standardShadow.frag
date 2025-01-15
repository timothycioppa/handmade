#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 FragNormal;
layout (location = 2) out vec3 FragPosition;
#define MAX_LIGHTS 4
in VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct time_data 
{ 
    float totalTime;
    float deltaTime;
    float cosTime;
    float sinTime;
};

struct Material 
{
    sampler2D mainTex;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 


struct light_data 
{ 
    int active;
    vec3 color;
    vec3 position;
    float intensity;
};

uniform time_data time;
uniform sampler2D unity_ShadowMap;
uniform vec3 unity_LightPosition;
uniform vec3 unity_CameraPosition;
uniform vec3 unity_CameraForward;
uniform vec3 lightColor;
uniform float lightPower;  
uniform Material material;

uniform light_data lights[MAX_LIGHTS];

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(unity_ShadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(unity_LightPosition - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(unity_ShadowMap, 0);
   
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(unity_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    if(projCoords.z > 1.0) 
    { 
        shadow = 0.0;
    }       
    return shadow;
}

float calculateFog(vec3 pos) 
{    
    float exposure = 5f;
    float _dist = abs(dot(pos - unity_CameraPosition, unity_CameraForward)) / length(unity_CameraForward); 
    float maxDist = 75.0f;
    float fog = exp(-(exposure / 100.0f) *  _dist);
    return max(0.15f, fog);
}




void main()
{          

    // vec2 tex_st = vec2(2.0f, 2.0f);
    // vec2 tex_offset = vec2(time.totalTime * 0.025f, time.totalTime * 0.025f);
    // vec2 texCoords = (fs_in.TexCoords + tex_offset) * tex_st;

    float shadowValue = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 textureColor = texture(material.mainTex,fs_in.TexCoords).rgb; 
    vec3 lightAmbientColor = lightPower * lightColor;
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(unity_CameraPosition - fs_in.FragPos);
    vec3 calculatedColor = vec3(0,0,0);

    int i = 0;

    for (int i = 0; i < MAX_LIGHTS; i++) 
    { 
        if (lights[i].active == 1) 
        { 
            vec3 dLight = lights[i].position - fs_in.FragPos;
            float invSqDist = 1.0f / (dLight.x * dLight.x + dLight.y * dLight.y + dLight.z * dLight.z);
            float dist = distance(lights[i].position, fs_in.FragPos);
            vec3 lightDir = normalize(dLight);                
            float diffuseValue = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diffuseValue * material.diffuse;          
            vec3 reflectDir = reflect(-lightDir, norm);  
            float specValue = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
            vec3 specular = specValue * lights[i].color;      
            calculatedColor += lights[i].intensity * (diffuse + specular) * invSqDist;
        }
    }

    float fog = calculateFog(fs_in.FragPos);
    float minShadow = 0.2f;
    float remappedShadow = minShadow + (1.0f - minShadow) * (1.0f - shadowValue);
    float multiplier = fog * remappedShadow;
    vec3 finalColor = multiplier * (lightAmbientColor + calculatedColor) * textureColor;

    FragColor = vec4(finalColor * multiplier, 1.0);
    FragNormal = 0.5f * (norm + 1.5f);
    FragPosition = fs_in.FragPos.xyz;
}