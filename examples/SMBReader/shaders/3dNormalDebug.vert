#version 460
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_ARB_shader_draw_parameters : require
#extension GL_GOOGLE_include_directive: require

#include "include/Math.iglsl"
#include "include/Mesh.iglsl"

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) uniform GlobalContext 
{
    mat4 view;
    mat4 proj;
    Frustum f;
    mat4 world;
} gs;

layout(set = 1, binding = 1) uniform texture2D Textures[];
layout(set = 1, binding = 0) uniform sampler samplerLinear;

layout(set = 2, binding = 0) readonly buffer PMBuffer 
{
    MeshDetails objects[];
} perModelBuffer;

layout(set = 2, binding = 1) readonly buffer InputVertices 
{
	uint8_t vertexData[];
} VertexData;

#include "include/MeshVertexFetch.iglsl"

layout(set = 2, binding = 2) uniform usamplerBuffer globalRenderableIndices;

layout(set = 2, binding = 3) readonly buffer RENDBuffer
{
    MeshRenderable renderables[];
} rends;

layout(set = 2, binding = 4) readonly buffer GeomDescBuffer
{
    GeometryDetails details[];
} geomDetails;

layout(set = 2, binding = 5) readonly buffer GeomInstBuffer
{
    GeometryRenderable geomRenderables[];
} geomRends;

void main() {
   
    uint renderableIndex = uint(texelFetch(globalRenderableIndices, gl_DrawID).r);

    MeshRenderable currentRenderable = rends.renderables[renderableIndex];

    GeometryRenderable lGeomRenderable = geomRends.geomRenderables[currentRenderable.geomInstIndex];

    GeometryDetails lGeomDetails = geomDetails.details[lGeomRenderable.geomDescIndex];

    MeshDetails modelData = perModelBuffer.objects[currentRenderable.meshIndex];

    uint comp = modelData.vertexComponents;

    uint stride = modelData.vertexStride;

    uint offset = (stride * uint(gl_VertexIndex/2)) + modelData.vertexByteOffset;
    
    vec3 normal = vec3(0.0);

    color = vec4(1.0, 1.0, 0.0, 1.0);

    mat4 meshWorld = lGeomRenderable.transform * currentRenderable.transform;

    if ((comp&COMPRESSED)==COMPRESSED)
    {
         if ((comp&BONES2)==BONES2)
        {
            offset += 4;
        }

        if ((comp & TEXTURES1) == TEXTURES1)
        {
            offset += 4;
        }

        if ((comp & TEXTURES2) == TEXTURES2)
        {
            offset += 4;
        }

        if ((comp & TEXTURES3) == TEXTURES3)
        {
            offset += 4;
        }

        if ((comp&NORMAL)==NORMAL)
        {
            normal = convertnormal(offset);

            offset += 4;
        }

        if ((comp&TANGENT)==TANGENT)
        {
            offset += 4;
        }

        if ((comp&COLOR)==COLOR)
        {
            offset += 4;
        }

        if ((comp & POSITION) == POSITION)
        {
            mat4 VP = gs.proj * gs.view;
            vec4 intPos = vec4(pack6decomp(offset, lGeomDetails.minMaxBox), 1.0f);
            gl_Position = VP * meshWorld * intPos;
        }
    } 
    else 
    {
         if ((comp&BONES2) == BONES2)
        {
            offset += 16;
        }

        if ((comp & TEXTURES1) == TEXTURES1)
        {
            offset += 8;
        }

        if ((comp & TEXTURES2) == TEXTURES2)
        {
            offset += 8;
        }

        if ((comp & TEXTURES3) == TEXTURES3)
        {
            offset += 8;
        }

        if ((comp&NORMAL) == NORMAL)
        {
            normal = ReconstructVEC4(offset).xyz;

            offset += 12;
        }

        if ((comp&TANGENT) == TANGENT)
        {
            offset += 16;
        }

        if ((comp&COLOR)==COLOR)
        {
            offset += 16;
        }

        if ((comp & POSITION) == POSITION)
        {
            mat4 MVP = gs.proj * gs.view * meshWorld;
            vec4 intPos = ReconstructVEC4(offset);
            gl_Position = MVP * intPos;
            offset += 16;
        }
    }
}