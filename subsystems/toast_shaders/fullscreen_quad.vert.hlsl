struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float2 texCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 vert_position : SV_POSITION;
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float2 texCoord : TEXCOORD0;
};


struct PushConstants
{
    uint samplerIndex;
    uint textureIndex;
};

[[vk::push_constant]] PushConstants pushData;

VSOutput main(VSInput p_input)
{
    VSOutput output = (VSOutput)0;

    output.vert_position = float4(p_input.position.xy, 0.0f, 1.0f);

    output.position =  output.vert_position.xyz;
    output.texCoord = p_input.texCoord;
//    output.texCoord.y *= -1.0f;

    return output;
}
