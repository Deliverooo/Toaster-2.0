#version 460

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 1, binding = 0) uniform sampler2D u_Texture;

const float offset = 1.0f / 900.0f;

void main()
{
    vec3 tintColour = vec3(1.0f, 1.0f, 1.0f);

    vec2 offsets[9] = vec2[](
    vec2(-offset, offset), // top-left
    vec2(0.0f, offset), // top-center
    vec2(offset, offset), // top-right
    vec2(-offset, 0.0f), // center-left
    vec2(0.0f, 0.0f), // center-center
    vec2(offset, 0.0f), // center-right
    vec2(-offset, -offset), // bottom-left
    vec2(0.0f, -offset), // bottom-center
    vec2(offset, -offset)// bottom-right
    );

    float kernel[9] = float[](

    -1, 0.5f, 1,
    -1, 0, 1,
    -1, 0.5f, 1
    );

    vec3 col = vec3(0.0f);

    for (int i = 0; i < 9; i++){
        col += vec3(texture(u_Texture, v_TexCoord + offsets[i])) * kernel[i];
    }

    o_Colour = vec4(col, 1.0f);
}