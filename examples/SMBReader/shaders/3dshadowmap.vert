#version 460
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_spirv_intrinsics : enable
#extension GL_GOOGLE_include_directive: require

#include "include/Math.iglsl"
#include "include/Mesh.iglsl"
#include "include/Lights.iglsl"

out gl_PerVertex {
    vec4  gl_Position;
    float gl_ClipDistance[4];
};

layout(set = 0, binding = 0) readonly buffer PMBuffer 
{
    MeshDetails objects[];
} perModelBuffer;

layout(set = 0, binding = 1) readonly buffer InputVertices 
{
	uint8_t vertexData[];
} VertexData;

#include "include/MeshVertexFetch.iglsl"

layout(set = 0, binding = 2) uniform usamplerBuffer globalRenderableIndices;

layout(set = 0, binding = 3) readonly buffer RENDBuffer
{
    MeshRenderable renderables[];
} rends;

layout(set = 0, binding = 4) uniform usamplerBuffer globalSMRenderableStart;
layout(set = 0, binding = 5) uniform usamplerBuffer globalSMPerRendIndices;

layout(set = 0, binding = 6) readonly buffer ShadowMap 
{
    ShadowMapViewProj viewProjs[];
} sm;

layout(set = 0, binding = 7) readonly buffer shadowViews
{
    ShadowMapView views[];
} sv;

layout(set = 0, binding = 8) readonly buffer GeomDescBuffer
{
    GeometryDetails details[];
} geomDetails;

layout(set = 0, binding = 9) readonly buffer GeomInstBuffer
{
    GeometryRenderable geomRenderables[];
} geomRends;

layout(set = 0, binding = 10) readonly buffer GLBuffer 
{
    LightSource objects[];
} lightBuffer;

layout(set = 0, binding = 11) uniform usamplerBuffer globalLightTypes;

void main() 
{    
    uint renderableIndex = uint(texelFetch(globalRenderableIndices, gl_DrawID).r);

    uint shadowMapBase = uint(texelFetch(globalSMRenderableStart, int(renderableIndex)).r);

    uint lightIndex = uint(texelFetch(globalSMPerRendIndices, int(shadowMapBase) + gl_InstanceIndex).r);

    uint shadowMapViewIndex = floatBitsToUint(lightBuffer.objects[lightIndex].direction.w);

    ShadowMapViewProj sViewProj = sm.viewProjs[shadowMapViewIndex];

    ShadowMapView viewSize = sv.views[shadowMapViewIndex];

    uint geomInstIndex = rends.renderables[renderableIndex].geomInstIndex;
    
    uint meshIndex = rends.renderables[renderableIndex].meshIndex;

    GeometryDetails lGeomDetails = geomDetails.details[geomRends.geomRenderables[geomInstIndex].geomDescIndex];

    uint comp = perModelBuffer.objects[meshIndex].vertexComponents;

    uint stride = perModelBuffer.objects[meshIndex].vertexStride;

    uint offset = (stride * gl_VertexIndex) + perModelBuffer.objects[meshIndex].vertexByteOffset;

    vec2 viewBase = 2.0 * vec2(viewSize.xOff, viewSize.yOff) - 1.0;

    vec4 scale = vec4(2.0 * viewSize.xScale, 2.0 * viewSize.yScale, 1.0, 1.0);

    mat4 meshWorld = geomRends.geomRenderables[geomInstIndex].transform * rends.renderables[renderableIndex].transform;

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
            mat4 MVP = sViewProj.shadowMapViewProj * meshWorld;
        
            vec4 intPos = vec4(pack6decomp(offset, lGeomDetails.minMaxBox), 1.0f);
            
            vec4 outPos = (MVP * intPos);

            gl_ClipDistance[0] = outPos.w + outPos.x;
            gl_ClipDistance[1] = outPos.w - outPos.x;
            gl_ClipDistance[2] = outPos.w + outPos.y;
            gl_ClipDistance[3] = outPos.w - outPos.y;

            vec2 s = 0.5 * scale.xy;

            gl_Position = vec4(s * outPos.xy + (s + viewBase) * outPos.w, scale.z * outPos.z, outPos.w);
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
            mat4 MVP = sViewProj.shadowMapViewProj * meshWorld;

            vec4 intPos = ReconstructVEC4(offset);

            vec4 outPos = (MVP * intPos);

            gl_ClipDistance[0] = outPos.w + outPos.x;
            gl_ClipDistance[1] = outPos.w - outPos.x;
            gl_ClipDistance[2] = outPos.w + outPos.y;
            gl_ClipDistance[3] = outPos.w - outPos.y;

            vec2 s = 0.5 * scale.xy;

            gl_Position = vec4(s * outPos.xy + (s + viewBase) * outPos.w, scale.z * outPos.z, outPos.w);
        }
    }
}