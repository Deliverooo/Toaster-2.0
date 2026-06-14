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

struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 invProj;
};

struct PushConstants
{
    vk::BufferPointer<Camera> cameraPtr;

    uint samplerId;
    uint skyboxMapId;
};

[[vk::push_constant]] PushConstants pushData;

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

    output.vert_position = float4(p_input.position.xy, 0.0f, 1.0f);

    float3x3 inv_view = (float3x3)Inverse(pushData.cameraPtr.Get().view);

    float4 view_pos = mul(pushData.cameraPtr.Get().invProj, output.vert_position);

    output.position =  mul(inv_view, view_pos.xyz).xyz;
    output.texCoord = p_input.texCoord;
    output.position.y *= -1.0f;

    return output;
}