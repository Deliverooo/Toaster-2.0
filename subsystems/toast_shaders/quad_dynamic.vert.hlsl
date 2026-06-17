struct VSInput
{
    [[vk::location(0)]] float4 position : POSITION0;
    [[vk::location(1)]] float3 colour : COLOR0;
    [[vk::location(2)]] float2 texCoord : TEXCOORD0;
    [[vk::location(3)]] float samplerIndex : SAMPLERINDEX0;
    [[vk::location(4)]] float texIndex : TEXINDEX0;
    [[vk::location(5)]] float tilingFactor : TILINGFACTOR0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    [[vk::location(0)]] float3 colour : COLOR0;
    [[vk::location(1)]] float2 texCoord : TEXCOORD0;
    [[vk::location(2)]] uint samplerIndex : SAMPLERINDEX0;
    [[vk::location(3)]] uint texIndex : TEXINDEX0;
    [[vk::location(4)]] float tilingFactor : TILINGFACTOR0;
};

struct Camera
{
    float4x4 view;
    float4x4 proj;
};

struct PushConstants
{
    vk::BufferPointer<Camera> cameraPtr;
};
[[vk::push_constant]] PushConstants pushData;

VSOutput main(VSInput p_input)
{
    VSOutput output = (VSOutput)0;

    float4 world_pos = p_input.position;
    float4 view_pos = mul(pushData.cameraPtr.Get().view, world_pos);
    output.position = mul(pushData.cameraPtr.Get().proj, view_pos);

    output.colour = p_input.colour;
    output.texCoord = p_input.texCoord;
    output.samplerIndex = (uint)p_input.samplerIndex;
    output.texIndex = (uint)p_input.texIndex;
    output.tilingFactor = p_input.tilingFactor;
//    output.texCoord.y *= -1.0f; // Silly Vulkan
    return output;
}
