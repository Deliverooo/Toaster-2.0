#version 450

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;

layout(push_constant) uniform PushConstants
{
    float time;
    vec2  res;
    vec2  mouse;
    vec3  cameraPos;
} pcs;

layout(binding = 1) uniform sampler2D albedoMap;

layout(location = 0) out vec4 o_FragColor;

// Lighting parameters
const vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
const vec3 lightColor = vec3(1.0, 0.98, 0.95);
const float ambientStrength = 0.15;
const float specularStrength = 0.5;
const float shininess = 32.0;

// Material color (can be replaced with texture sampling later)
const vec3 baseColor = vec3(0.8, 0.8, 0.8);

void main()
{
    vec3 normal = normalize(v_Normal);
    vec3 viewDir = normalize(pcs.cameraPos - v_WorldPos);

    // Ambient
    vec3 ambient = ambientStrength * lightColor * texture(albedoMap, v_TexCoord).rgb;

    // Diffuse (Lambertian)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff  * lightColor * texture(albedoMap, v_TexCoord).rgb;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine lighting
    vec3 result = (ambient + diffuse + specular) * baseColor;

    // Simple tone mapping
    result = result / (result + vec3(1.0));

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    o_FragColor = vec4(result, 1.0);
}
