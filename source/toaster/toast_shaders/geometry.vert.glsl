#version 460

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_WorldPos;
layout(location = 1) out vec3 v_Position;
layout(location = 2) out vec2 v_TexCoord;
layout(location = 3) out vec3 v_Normal;
layout(location = 4) out mat3 v_WorldNormals;

layout(std140, set = 1, binding = 1) uniform Camera
{
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_InvProj;
};

layout (push_constant) uniform Transform
{
    mat4 model;
} _Transform_;

invariant gl_Position;
void main()
{
    vec4 world_position = _Transform_.model * vec4(a_Position, 1.0f);
    vec4 view_position = u_View * world_position;

    gl_Position = u_Proj * view_position;

    v_WorldPos = world_position.xyz;
    v_Position = a_Position;

    v_TexCoord = a_TexCoord;
    v_Normal = mat3(transpose(inverse(_Transform_.model))) * a_Normal;
    v_WorldNormals = mat3(_Transform_.model) * mat3(a_Tangent, a_Bitangent, a_Normal);
}