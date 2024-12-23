#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D unity_ShadowMap;
uniform vec3 unity_LightPosition;
uniform vec3 unity_CameraPosition;
uniform vec3 lightColor;
uniform float lightPower;

struct Material 
{
    sampler2D mainTex;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
  
uniform Material material;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(unity_ShadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(unity_LightPosition - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
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
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0) 
    { 
        shadow = 0.0;
    }       
    return shadow;
}

 


void main()
{          
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 texColor = texture(material.mainTex, fs_in.TexCoords).rgb; 
    vec3 ambient = lightColor * texColor;   

    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(unity_LightPosition - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor *  (diff * material.diffuse); 
    vec3 viewDir = normalize(unity_CameraPosition - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular);          
    vec3 result = (ambient + (1.0f - shadow) * ( diffuse + specular))  * (lightPower / 100.0f);
 
    FragColor = vec4(result, 1.0);
}