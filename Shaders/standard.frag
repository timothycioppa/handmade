#version 330 core

// vertex inputs
in vec4 positionWS;
in vec3 normalCameraSpace;
in vec3 eyeDirectionCameraSpace;
in vec3 lightDirectionCameraSpace;
in vec2 uv;


out vec4 FragColor;
uniform sampler2D samplerMain;
uniform mat4 unity_MV;
uniform float totalTime;
uniform float deltaTime;
uniform float cosTime;
uniform float sinTime;
uniform vec3 lightPosWS;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform float specularSharpness;
uniform float lightPower;

void main()
{

    vec3 diffuseColor = texture( samplerMain, uv ).xyz;

    float dist = length(lightPosWS - positionWS.xyz);
    float InvSqDist = 1.0f / (dist * dist);
    vec3 normal = normalize(normalCameraSpace);
    vec3 lightDir = normalize(lightDirectionCameraSpace);
    float diffuseValue = clamp(dot(normal, lightDir), 0.0f, 1.0f);

    vec3 diffuse = diffuseValue * diffuseColor * InvSqDist;

    vec3 eyeDir = normalize(eyeDirectionCameraSpace);
    vec3 halfVector = reflect(-lightDir, normal);
    float specValue = clamp(dot(eyeDir, halfVector), 0, 1);
    vec3 specular = specularColor * pow(specValue, specularSharpness) * InvSqDist;

    vec3 color = lightColor * lightPower * (diffuse + specular) + ambientColor;
    vec3 gammaCorrected = pow(color, vec3(1.0f / 2.2f));

    FragColor = vec4(gammaCorrected, 1.0f);
}