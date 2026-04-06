#version 460

layout(location = 0) in vec3 v_Colour;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Colour;

layout(set = 0, binding = 1) uniform sampler2D u_Texture;
layout(set = 0, binding = 2) uniform sampler2D u_Texture2;

layout(push_constant) uniform FrameData
{
    vec2 res;
    float time;
} frameData;

float distanceToMandelbrot(in vec2 c)
{
    // iterate
    float di =  1.0;
    vec2 z  = vec2(0.0);
    float m2 = 0.0;
    vec2 dz = vec2(0.0);
    for (int i=0; i<300; i++)
    {
        if (m2>1024.0) { di=0.0; break; }

        // Z' -> 2·Z·Z' + 1
        dz = 2.0*vec2(z.x*dz.x-z.y*dz.y, z.x*dz.y + z.y*dz.x) + vec2(1.0, 0.0);

        // Z -> Z² + c
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;

        m2 = dot(z, z);
    }

    // distance
    // d(c) = |Z|·log|Z|/|Z'|
    float d = 0.5*sqrt(dot(z, z)/dot(dz, dz))*log(dot(z, z));
    //if( di>0.5 ) d=0.0;

    return d;
}


void main()
{
    vec2 p = (2.0f * gl_FragCoord.xy - frameData.res.xy)/frameData.res.y;

    // animation
    float tz = 0.5 - 0.5*cos(0.225*frameData.time);
    float zoo = pow(0.5, 13.0*tz);
    vec2 c = vec2(-0.05, .6805) + p*zoo;

    // distance to Mandelbrot
    float d = distanceToMandelbrot(c);

    // do some soft coloring based on distance
    d = clamp(pow(4.0*d/zoo, 0.2), 0.0, 1.0);
    //d =pow(d,.1);
    //d = 1.0-1.0/(1.0+1000.0*d);


    vec3 col = vec3(d) * texture(u_Texture2, v_TexCoord).rgb;

    o_Colour = vec4(col, 1.0f);
}