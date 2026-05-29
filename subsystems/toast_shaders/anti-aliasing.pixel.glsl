#version 460

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 1, binding = 0) uniform sampler2D u_Texture;

#define FXAA_SPAN_MAX     8.0
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_REDUCE_MIN   (1.0 / 128.0)

void main()
{
    // Determine the pixel size of the screen texture
    vec2 texelSize = 1.0 / textureSize(u_Texture, 0);

    // Sample the center pixel and 4 immediate neighbors
    vec3 rgbNW = texture(u_Texture, v_TexCoord + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 rgbNE = texture(u_Texture, v_TexCoord + vec2(1.0, -1.0) * texelSize).rgb;
    vec3 rgbSW = texture(u_Texture, v_TexCoord + vec2(-1.0, 1.0) * texelSize).rgb;
    vec3 rgbSE = texture(u_Texture, v_TexCoord + vec2(1.0, 1.0) * texelSize).rgb;
    vec3 rgbM  = texture(u_Texture, v_TexCoord).rgb;

    // Convert colors to luminance (brightness)
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM, luma);

    // Find min and max luminance among neighbors
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    // Edge direction calculation
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
    max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
    dir * rcpDirMin)) * texelSize;

    // Blur along the detected edge direction
    vec3 rgbA = 0.5 * (
    texture(u_Texture, v_TexCoord + dir * (1.0 / 3.0 - 0.5)).rgb +
    texture(u_Texture, v_TexCoord + dir * (2.0 / 3.0 - 0.5)).rgb);

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
    texture(u_Texture, v_TexCoord + dir * -0.5).rgb +
    texture(u_Texture, v_TexCoord + dir * 0.5).rgb);

    float lumaB = dot(rgbB, luma);

    // Safety check to ensure we do not over-blur non-edges
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        o_Colour = vec4(rgbA, 1.0);
    } else {
        o_Colour = vec4(rgbB, 1.0);
    }


    //    vec3 tex_colour = texture(u_Texture, v_TexCoord).rgb;

    //    o_Colour = vec4(tex_colour.rg, 0.0f, 1.0f);
}