#version 460
#extension GL_GOOGLE_include_directive: require
#include "common.glslh"

struct Vertex
{
    vec4 position;
    vec3 normal;
    float _padd1[1];
    vec3 tangent;
    float _padd2[1];
    vec3 bitangent;
    float _padd3[1];
    vec2 texCoord;
    float _padd4[2];
};

layout (std430, buffer_reference) readonly buffer VertexBuffer { Vertex vertices[]; };

struct Camera
{
    mat4 view;
    mat4 proj;
    mat4 invProj;
};

layout (descriptor_heap) uniform CameraHeap
{
    Camera data;
} cameraHeap[];

layout (push_constant) uniform Constants
{

    mat4 meshTransform;

    VertexBuffer vbo;

    uint64_t pointLightsPtr;

    uint materialIndex;

    uint cameraIndex;
    uint sceneDataIndex;

    uint samplerIndex;
    uint diffuseIrradianceMapIndex;
    uint specularIrradianceMapIndex;

    uint BRDFLUTSamplerIndex;
    uint BRDFLUT;
} pcs;

layout (location = 0) out vec3 o_WorldPos;
layout (location = 1) out vec3 o_Normal;
layout (location = 2) out vec2 o_TexCoord;
layout (location = 3) out mat3 o_TBN;

void main()
{
    Vertex v_in = pcs.vbo.vertices[gl_VertexIndex];

    Camera camera = cameraHeap[nonuniformEXT(pcs.cameraIndex)].data;

    vec4 world_pos = pcs.meshTransform * v_in.position;
    vec4 view_pos = camera.view * world_pos;

    gl_Position = camera.proj * view_pos;

    o_WorldPos = world_pos.xyz;

    // Ts has to be world Normals!!
    o_Normal = mat3(transpose(inverse(pcs.meshTransform))) * v_in.normal;
    o_TexCoord = v_in.texCoord;

    o_TBN = mat3(pcs.meshTransform) * mat3(v_in.tangent, v_in.bitangent, v_in.normal);
}
