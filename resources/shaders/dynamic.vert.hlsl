struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float2 texCoord : TEXCOORD0;
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

struct Transform
{
  column_major    float4x4 model;
};

struct PushConstants
{
    uint textureIndex;
    uint samplerIndex;

    vk::BufferPointer<Camera> cameraPtr;

    // float4x4 transform;
    // vk::BufferPointer<Transform> transformPtr;
};

[[vk::push_constant]] PushConstants pushData;



VSOutput main(VSInput p_input)
{
    VSOutput output = (VSOutput)0;

    // float4x4 view_proj = mul(pushData.cameraPtr.Get().view, pushData.cameraPtr.Get().proj);

    float4 world_pos = float4(p_input.position.xy, 0.0f, 1.0f);
    float4 view_pos = mul(pushData.cameraPtr.Get().view, world_pos);
    output.position = mul(pushData.cameraPtr.Get().proj, view_pos);
    output.texCoord = p_input.texCoord;
    output.texCoord.y *= -1.0f;

    return output;
}
