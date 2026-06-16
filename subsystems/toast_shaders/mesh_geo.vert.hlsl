struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float3 normal : NORMAL0;
    [[vk::location(2)]] float3 tangent : TANGENT0;
    [[vk::location(3)]] float3 bitangent : BITANGENT0;
    [[vk::location(4)]] float2 texCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 vert_position : SV_POSITION;
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float3 normal : NORMAL0;
    [[vk::location(2)]] float2 texCoord : TEXCOORD0;
    [[vk::location(3)]] float3x3 worldNormals : NORMALMATRIX0;
};

struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 invProj;
};

struct DirectionalLight
{
    float4 direction;
    float3 radiance;
    float multiplier;
};

struct DirectionalLights
{
    uint count;
    DirectionalLight lights[4];
};

struct PointLight
{
    float4 position;
    float3 radiance;
    float multiplier;
};

struct PointLights
{
    uint count;
    PointLight lights[128];
};

struct SceneData
{
   float3 cameraPos;
   float _padd;
};

struct PushConstants
{
    vk::BufferPointer<Camera> cameraPtr;
    vk::BufferPointer<DirectionalLights> directionalLightsPtr;
    vk::BufferPointer<PointLights> pointLightsPtr;
    vk::BufferPointer<SceneData> sceneDataPtr;

    float4x4 modelMatrix;

    float4 albedoColour;

    uint samplerIndex;

    uint albedoMap;
    uint normalMap;
    uint hasNormalMap;

    float roughness;
    float metalness;

    float _padd[2];
};

[[vk::push_constant]] PushConstants pushData;

float3x3 Inverse(float3x3 m) {
    float Det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    float InvDet = 1.0f / Det;

    float3x3 Inv;
    Inv[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * InvDet;
    Inv[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * InvDet;
    Inv[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * InvDet;
    Inv[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * InvDet;
    Inv[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * InvDet;
    Inv[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * InvDet;
    Inv[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * InvDet;
    Inv[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * InvDet;
    Inv[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * InvDet;

    return Inv;
}


float4x4 Inverse(float4x4 m)
{
    float4x4 inv;

    inv[0][0] = m[1][1]  * m[2][2]  * m[3][3]  - m[1][1]  * m[2][3]  * m[3][2]  -
                m[2][1]  * m[1][2]  * m[3][3]  + m[2][1]  * m[1][3]  * m[3][2]  +
                m[3][1]  * m[1][2]  * m[2][3]  - m[3][1]  * m[1][3]  * m[2][2];

    inv[1][0] = -m[1][0] * m[2][2]  * m[3][3]  + m[1][0] * m[2][3]  * m[3][2]  +
                 m[2][0] * m[1][2]  * m[3][3]  - m[2][0] * m[1][3]  * m[3][2]  -
                 m[3][0] * m[1][2]  * m[2][3]  + m[3][0] * m[1][3]  * m[2][2];

    inv[2][0] =  m[1][0] * m[2][1]  * m[3][3]  - m[1][0] * m[2][3]  * m[3][1]  -
                 m[2][0] * m[1][1]  * m[3][3]  + m[2][0] * m[1][3]  * m[3][1]  +
                 m[3][0] * m[1][1]  * m[2][3]  - m[3][0] * m[1][3]  * m[2][1];

    inv[3][0] = -m[1][0] * m[2][1]  * m[3][2]  + m[1][0] * m[2][2]  * m[3][1]  +
                 m[2][0] * m[1][1]  * m[3][2]  - m[2][0] * m[1][2]  * m[3][1]  -
                 m[3][0] * m[1][1]  * m[2][2]  + m[3][0] * m[1][2]  * m[2][1];

    inv[0][1] = -m[0][1] * m[2][2]  * m[3][3]  + m[0][1] * m[2][3]  * m[3][2]  +
                 m[2][1] * m[0][2]  * m[3][3]  - m[2][1] * m[0][3]  * m[3][2]  -
                 m[3][1] * m[0][2]  * m[2][3]  + m[3][1] * m[0][3]  * m[2][2];

    inv[1][1] =  m[0][0] * m[2][2]  * m[3][3]  - m[0][0] * m[2][3]  * m[3][2]  -
                 m[2][0] * m[0][2]  * m[3][3]  + m[2][0] * m[0][3]  * m[3][2]  +
                 m[3][0] * m[0][2]  * m[2][3]  - m[3][0] * m[0][3]  * m[2][2];

    inv[2][1] = -m[0][0] * m[2][1]  * m[3][3]  + m[0][0] * m[2][3]  * m[3][1]  +
                 m[2][0] * m[0][1]  * m[3][3]  - m[2][0] * m[0][3]  * m[3][1]  -
                 m[3][0] * m[0][1]  * m[2][3]  + m[3][0] * m[0][3]  * m[2][1];

    inv[3][1] =  m[0][0] * m[2][1]  * m[3][2]  - m[0][0] * m[2][2]  * m[3][1]  -
                 m[2][0] * m[0][1]  * m[3][2]  + m[2][0] * m[0][2]  * m[3][1]  +
                 m[3][0] * m[0][1]  * m[2][2]  - m[3][0] * m[0][2]  * m[2][1];

    inv[0][2] =  m[0][1] * m[1][2]  * m[3][3]  - m[0][1] * m[1][3]  * m[3][2]  -
                 m[1][1] * m[0][2]  * m[3][3]  + m[1][1] * m[0][3]  * m[3][2]  +
                 m[3][1] * m[0][2]  * m[1][3]  - m[3][1] * m[0][3]  * m[1][2];

    inv[1][2] = -m[0][0] * m[1][2]  * m[3][3]  + m[0][0] * m[1][3]  * m[3][2]  +
                 m[1][0] * m[0][2]  * m[3][3]  - m[1][0] * m[0][3]  * m[3][2]  -
                 m[3][0] * m[0][2]  * m[1][3]  + m[3][0] * m[0][3]  * m[1][2];

    inv[2][2] =  m[0][0] * m[1][1]  * m[3][3]  - m[0][0] * m[1][3]  * m[3][1]  -
                 m[1][0] * m[0][1]  * m[3][3]  + m[1][0] * m[0][3]  * m[3][1]  +
                 m[3][0] * m[0][1]  * m[1][3]  - m[3][0] * m[0][3]  * m[1][1];

    inv[3][2] = -m[0][0] * m[1][1]  * m[3][2]  + m[0][0] * m[1][2]  * m[3][1]  +
                 m[1][0] * m[0][1]  * m[3][2]  - m[1][0] * m[0][2]  * m[3][1]  -
                 m[3][0] * m[0][1]  * m[1][2]  + m[3][0] * m[0][2]  * m[1][1];

    inv[0][3] = -m[0][1] * m[1][2]  * m[2][3]  + m[0][1] * m[1][3]  * m[2][2]  +
                 m[1][1] * m[0][2]  * m[2][3]  - m[1][1] * m[0][3]  * m[2][2]  -
                 m[2][1] * m[0][2]  * m[1][3]  + m[2][1] * m[0][3]  * m[1][2];

    inv[1][3] =  m[0][0] * m[1][2]  * m[2][3]  - m[0][0] * m[1][3]  * m[2][2]  -
                 m[1][0] * m[0][2]  * m[2][3]  + m[1][0] * m[0][3]  * m[2][2]  +
                 m[2][0] * m[0][2]  * m[1][3]  - m[2][0] * m[0][3]  * m[1][2];

    inv[2][3] = -m[0][0] * m[1][1]  * m[2][3]  + m[0][0] * m[1][3]  * m[2][1]  +
                 m[1][0] * m[0][1]  * m[2][3]  - m[1][0] * m[0][3]  * m[2][1]  -
                 m[2][0] * m[0][1]  * m[1][3]  + m[2][0] * m[0][3]  * m[1][1];

    inv[3][3] =  m[0][0] * m[1][1]  * m[2][2]  - m[0][0] * m[1][2]  * m[2][1]  -
                 m[1][0] * m[0][1]  * m[2][2]  + m[1][0] * m[0][2]  * m[2][1]  +
                 m[2][0] * m[0][1]  * m[1][2]  - m[2][0] * m[0][2]  * m[1][1];

    float det = m[0][0] * inv[0][0] + m[0][1] * inv[1][0] + m[0][2] * inv[2][0] + m[0][3] * inv[3][0];

    if (det == 0.0f)
        return m; // Return original matrix if it cannot be inverted

    det = 1.0f / det;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            inv[i][j] = inv[i][j] * det;
        }
    }

    return inv;
}


VSOutput main(VSInput p_input)
{
    VSOutput output = (VSOutput)0;

    float4 world_pos = mul(float4(p_input.position.xyz, 1.0f), pushData.modelMatrix);
    float4 view_pos = mul(pushData.cameraPtr.Get().view, world_pos);
    output.vert_position = mul(pushData.cameraPtr.Get().proj, view_pos);

    output.position = world_pos.xyz;

    output.normal = mul((float3x3)transpose(Inverse(pushData.modelMatrix)), p_input.normal);
    output.worldNormals = mul(float3x3(p_input.tangent, p_input.bitangent, p_input.normal) , (float3x3)pushData.modelMatrix);


    output.texCoord = p_input.texCoord;

    return output;
}
