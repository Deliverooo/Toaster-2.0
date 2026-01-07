#version 450

layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;

layout(push_constant) uniform PushConstants
{
    float time;
    vec2  res;
    vec2  mouse;
    vec3  cameraPos;
} pcs;

layout(location = 0) out vec4 o_fragColour;

#define FOV 0.7f


vec3 palette(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d)
{
    return a + b*cos(6.283185*(c*t+d));
}

float sMin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0f) / k;
    return min(a, b) - pow(h, 3) * k * (1.0f / 6.0f);
}

float sphereSDF(vec3 p, float r)
{
    return length(p) - r;
}

float boxSDF(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
}

mat2 rot2D(float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c);
}

float map(vec3 p)
{
    p.z += pcs.time * 0.5f;

    p = fract(p) - 0.5f;
    float box = boxSDF(p, vec3(0.1f));

    return box;
}

void main()
{


    vec2 uv = (gl_FragCoord.xy * 2.0f - pcs.res.xy) / pcs.res.y;
    uv.y *= -1.0f;

    vec3 ro = vec3(0.0f, 0.0f, -3.0f);
    vec3 rd = normalize(vec3(uv, 1.0f));
    vec3 col = vec3(0.0f);

    float t = 0.0f;


    //    ro.yz *= rot2D(-pcs.cameraPos.y);
    //    rd.yz *= rot2D(-pcs.cameraPos.y);

    //    ro.xz *= rot2D(-pcs.cameraPos.x);
    //    rd.xz *= rot2D(-pcs.cameraPos.x);


    for (int i = 0; i < 100; i++)
    {

        vec3 p = ro + rd * t;

        float d = map(p);

        t += d;

        if (d < 0.001f || t > 100.0f) break;
    }

    vec3 col1 = vec3(0.5f, 0.5f, 0.5f);
    vec3 col2 = vec3(0.5f, 0.5f, 0.5f);
    vec3 col3 = vec3(1.0f, 1.0f, 1.0f);
    vec3 col4 = vec3(0.264f, 0.416f, 0.557f);
    col = palette(t, col1, col2, col3, col4);

    o_fragColour = vec4(col, 1.0f);
}