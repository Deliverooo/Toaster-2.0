#version 460

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 1, binding = 0) uniform sampler2D u_Texture;


layout(push_constant) uniform Constants
{
    vec2 res;
} u_Constants;

void main()
{
//    vec2 uv = (2.0f * gl_FragCoord.xy - u_Constants.res) / u_Constants.res.y;
//
//    float y = sqrt(1.0f - pow(uv.x, 2.0f));
//
//    float thickness = 0.01f;
//    float graph = smoothstep(thickness, 0.0f, abs(uv.y - y));
//
//    vec3 final_colour = vec3(graph);
    vec3 tex_colour = texture(u_Texture, v_TexCoord).rgb;

    o_Colour = vec4(tex_colour, 1.0f);
}