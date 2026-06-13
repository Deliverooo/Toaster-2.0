struct PSInput
{
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
};

[[vk::push_constant]] PushConstants pushData;

float4 main(PSInput p_input) : SV_TARGET
{

    Texture2D<float4> texture = ResourceDescriptorHeap[pushData.textureIndex];
    SamplerState tex_sampler = SamplerDescriptorHeap[pushData.samplerIndex];

    float4 out_colour = float4(1.0f, 0.0f, 0.0f, 1.0f);
    
    out_colour = texture.Sample(tex_sampler, p_input.texCoord);

    return out_colour;
}

// void main()
// {
// //    uint tex_index = (pc.imageArrayOffset / IMAGE_STRIDE) + pcs.textureIndex;
//     vec4 texture_colour = texture(sampler2D(globalTextures[pcs.textureIndex], globalSamplers[pcs.samplerIndex]), v_TexCoord);

//     o_Colour = texture_colour;
// //    o_Colour = vec4(v_TexCoord, 0.0f, 1.0f);
// }

