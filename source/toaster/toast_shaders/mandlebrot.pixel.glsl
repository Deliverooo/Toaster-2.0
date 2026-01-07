#version 450

#define PI 3.1415926f

#define SPRITE_DEC(x, i)    mod(floor(i / pow(4.0, mod(x, 8.0))), 4.0)
#define SPRITE_DEC2(x, i) mod(floor(i / pow(4.0, mod(x, 11.0))), 4.0)
#define RGB(r, g, b) vec3(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0)

const float MARIO_SPEED     = 89.0;
const float GOOMBA_SPEED = 32.0;
const float INTRO_LENGTH = 2.0;


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

float distanceToMandelbrot( in vec2 c )
{
    // iterate
    float di =  1.0;
    vec2 z  = vec2(0.0);
    float m2 = 0.0;
    vec2 dz = vec2(0.0);
    for( int i=0; i<300; i++ )
    {
        if( m2>1024.0 ) { di=0.0; break; }

        // Z' -> 2·Z·Z' + 1
        dz = 2.0*vec2(z.x*dz.x-z.y*dz.y, z.x*dz.y + z.y*dz.x) + vec2(1.0,0.0);

        // Z -> Z² + c
        z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;

        m2 = dot(z,z);
    }

    // distance
    // d(c) = |Z|·log|Z|/|Z'|
    float d = 0.5*sqrt(dot(z,z)/dot(dz,dz))*log(dot(z,z));
    //if( di>0.5 ) d=0.0;

    return d;
}


void main()
{

    vec2 p = (2.0f * gl_FragCoord.xy - pcs.res.xy)/pcs.res.y;

    // animation
    float tz = 0.5 - 0.5*cos(0.225*pcs.time);
    float zoo = pow( 0.5, 13.0*tz );
    vec2 c = vec2(-0.05,.6805) + p*zoo;

    // distance to Mandelbrot
    float d = distanceToMandelbrot(c);

    // do some soft coloring based on distance
    d = clamp( pow(4.0*d/zoo,0.2), 0.0, 1.0 );
    //d =pow(d,.1);
    //d = 1.0-1.0/(1.0+1000.0*d);


    vec3 col = vec3(d);


    o_fragColour = vec4(col, 1.0f);
}