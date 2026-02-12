#version 460

layout(location = 0) in vec2 v_TexCoord;

uniform sampler2D u_AlbedoMap;

uniform vec3 u_AlbedoColour;

layout(location = 0) out vec4 o_FragColour;

void main()
{
    vec3 diffuse = texture(u_AlbedoMap, v_TexCoord).rgb * u_AlbedoColour;

    o_FragColour = vec4(diffuse, 1.0f);
}
