#version 460

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

invariant gl_Position;

void main()
{
    gl_Position = vec4(a_Position.xy, 0.0f, 1.0f);

    v_TexCoord = a_TexCoord;
}