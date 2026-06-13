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
    float4 position : SV_POSITION;
    [[vk::location(0)]] float2 texCoord : TEXCOORD0;
};

struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 invProj;
};

struct PushConstants
{
    uint textureIndex;
    uint samplerIndex;

    vk::BufferPointer<Camera> cameraPtr;

    float4x4 modelMatrix;
};

[[vk::push_constant]] PushConstants pushData;

VSOutput main(VSInput p_input)
{
    VSOutput output = (VSOutput)0;

    float4 world_pos = mul(float4(p_input.position.xyz, 1.0f), pushData.modelMatrix);
    float4 view_pos = mul(pushData.cameraPtr.Get().view, world_pos);
    output.position = mul(pushData.cameraPtr.Get().proj, view_pos);
    output.texCoord = p_input.texCoord;

    return output;
}
