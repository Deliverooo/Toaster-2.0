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

        // Calculate UV coordinates based on vertex ID (0, 1, or 2)
    // output.texCoord = float2((id << 1) & 2, id & 2);
    
    // Convert UVs (0 to 1) to Clip Space Positions (-1 to 1)
    // output.position = float4(output.texCoord */ float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
 

    return output;
}
