#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive: require

#include "include/Math.iglsl"
#include "include/Mesh.iglsl"
#include "include/Lights.iglsl"

layout(location = 0) in vec4 worldPosition;
layout(location = 1) in vec2 texCoords[8];
layout(location = 9) in vec3 inNormal;
layout(location = 10) in vec4 vertColor;
layout(location = 11) flat in uint renderableIndex;
layout(location = 12) in vec4 inTangent;
layout(location = 0) out vec4 outColor;

struct UniformGrid
{
    vec4 max;
    vec4 min;
	uint numberOfDivisions;
    uint currentFrame;
};

layout(push_constant) uniform ShadowImageConstants 
{
    UniformGrid grid;
} sic;

layout(set = 0, binding = 0) uniform GlobalContext 
{
    mat4 view;
    mat4 proj;
    Frustum f;
    mat4 world;
} gs;

layout(set = 1, binding = 0) uniform sampler samplerLinear;

layout(set = 1, binding = 1) readonly buffer ShadowMap 
{
    ShadowMapViewProj viewProjs[];
} sm;

layout(set = 1, binding = 2) readonly buffer shadowViews
{
    ShadowMapView views[];
} sv;

layout(set = 1, binding = 3) uniform samplerShadow shadowSampler;

layout(set = 1, binding = 4) uniform texture2D DepthTextures[3];

layout(set = 1, binding = 5) uniform texture2D Textures[];

layout(set = 2, binding = 3) readonly buffer GLBuffer 
{
    LightSource objects[];
} lightBuffer;

layout(set = 2, binding = 4) uniform usamplerBuffer globalLightTypes;

layout(set = 2, binding = 5) uniform MaterialContext 
{
   Material matsData[50];
} mats;

layout(set = 2, binding = 6) readonly buffer RENDBuffer 
{
    MeshRenderable renderables[];
} rends;

layout(set = 2, binding = 7) uniform usamplerBuffer globalMaterialIndices;

layout(set = 2, binding = 8) readonly buffer BLENDBuffer 
{
    BlendDetails details[];
} blends;

layout(set = 2, binding = 9) uniform usamplerBuffer globalBlendIndices;

layout(set = 2, binding = 12) uniform usamplerBuffer globalLightOffsets;
layout(set = 2, binding = 13) uniform usamplerBuffer globalLightCounts;
layout(set = 2, binding = 14) uniform usamplerBuffer globalLightAllocations;

uint GetPointCubeFace(vec3 v) {
    vec3 vAbs = abs(v);

    uint faceIndex = 0;
 
    if (vAbs.z >= vAbs.x && vAbs.z >= vAbs.y) 
    {
        faceIndex = v.z < 0.0 ? 5 : 4;
    } 
    else if (vAbs.y >= vAbs.x) 
    {
        faceIndex = v.y < 0.0 ? 3 : 2;
    } 
    else 
    {
        faceIndex = v.x < 0.0 ? 1 : 0;
    }
    
    return faceIndex;
}

float ShadowContribution(uvec2 lightViewIndex, vec3 normal, vec3 lightingDir)
{
    ShadowMapViewProj sViewProj = sm.viewProjs[lightViewIndex.x + lightViewIndex.y];
    ShadowMapView sViewRange = sv.views[lightViewIndex.x + lightViewIndex.y];

    vec4 smTransCoords = sViewProj.shadowMapViewProj * worldPosition;

    vec3 projCoords = smTransCoords.xyz / smTransCoords.w;

    if (any(lessThan(projCoords.xy, vec2(-1.0))) || any(greaterThan(projCoords.xy, vec2(1.0))) || projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0;
    }

    vec2 projCoordsPair = projCoords.xy * 0.5 + 0.5;

    vec2 sViewOff = vec2(sViewRange.xOff, sViewRange.yOff);
    vec2 sViewScale = vec2(sViewRange.xScale, sViewRange.yScale);

    projCoordsPair  = (sViewScale * projCoordsPair) + sViewOff;

    float bias = max(0.05 * (1.0 - dot(normal, lightingDir)), 0.005);

    float currentDepth = projCoords.z;

    return texture(
        sampler2DShadow(DepthTextures[sic.grid.currentFrame], shadowSampler),
        vec3(projCoordsPair, currentDepth - bias)
    );
}

vec4 DoLights(vec3 normal, vec4 color, uint renderableIndex, vec3 specularBase, float shininess)
{
    vec3 camPos = gs.world[3].xyz;

    vec3 DiffuseColor = vec3(0.0);

    vec3 SpecularColor = vec3(0.0);

    vec3 AmbientColor = vec3(0.0);

    vec3 viewPosToWorldPos = normalize(camPos - worldPosition.xyz);

    vec3 range = sic.grid.max.xyz - sic.grid.min.xyz;

    uvec3 inGridPos = uvec3(round(range/(worldPosition.xyz - sic.grid.min.xyz)));

    uint gridIndex = (inGridPos.x * sic.grid.numberOfDivisions * sic.grid.numberOfDivisions) + (inGridPos.y * sic.grid.numberOfDivisions) + inGridPos.z;
  
    uint lightOffsets =  texelFetch(globalLightOffsets, int(gridIndex)).r;
    uint lightCounts =  texelFetch(globalLightCounts, int(gridIndex)).r;

    uint dc = GET_LIGHT_TYPE_DIRECTIONAL_COUNT(lightCounts);
    uint poc = GET_LIGHT_TYPE_POINT_COUNT(lightCounts);
    uint sc = GET_LIGHT_TYPE_SPOT_COUNT(lightCounts);

    uint lightCount = dc+poc+sc;

    for (uint lightIter = 0; lightIter<lightCount; lightIter++)
    {
        uint currentLightIndex = texelFetch(globalLightAllocations, int(lightOffsets+lightIter)).r;

        LightSource light = lightBuffer.objects[currentLightIndex];

        uint lType = uint(texelFetch(globalLightTypes, int(currentLightIndex)).r);

        vec3 lightColor = light.color.xyz/255.0;

        float diffuseIntensity = light.color.w;

        float specularIntensity = light.direction.w;

        vec3 lightDirection = light.direction.xyz;

        uint lightViewIndex = floatBitsToUint(light.direction.w);
        
        if (lType == LIGHT_TYPE_DIRECTIONAL)
        {
            vec3 lightingDir = normalize(-lightDirection); 

            vec3 H = normalize(lightingDir + viewPosToWorldPos);

            float diffuse = max(dot(normal, lightingDir), 0.0);

            float specFactor = max(dot(normal, H), 0.0);

            specFactor = (diffuse > 0.0) ? pow(specFactor, shininess) : 0.0;

            float shadowColor = ShadowContribution(uvec2(lightViewIndex, 0), normal, lightingDir);

            DiffuseColor += lightColor * diffuse * diffuseIntensity * shadowColor;

            AmbientColor += light.ancillary.xyz * shadowColor;

            SpecularColor += lightColor * specularBase * specFactor * specularIntensity * shadowColor;
        }
        else if (lType == LIGHT_TYPE_POINT)
        {
            vec3 lightingDir = light.pos.xyz - worldPosition.xyz;

            float distance = length(lightingDir);

            lightingDir = normalize(lightingDir);

            float diffuse = max(dot(normal, lightingDir), 0.0);

            vec3 H = normalize(lightingDir + viewPosToWorldPos);

            float attenuation = (1.0/(.01 + (.25 * distance) + (.67 * distance * distance)));

            float specFactor = max(dot(normal, H), 0.0);

            specFactor = (diffuse > 0.0) ? pow(specFactor, shininess) : 0.0;

            float shadowColor = ShadowContribution(uvec2(lightViewIndex, GetPointCubeFace(worldPosition.xyz - light.pos.xyz)), normal, lightingDir);

            AmbientColor += light.ancillary.xyz * attenuation;

            DiffuseColor += lightColor * diffuse * diffuseIntensity * attenuation * shadowColor;

            SpecularColor += lightColor * specularBase * specFactor * specularIntensity * attenuation * shadowColor;
        }
        else if (lType == LIGHT_TYPE_SPOT)
        {
            vec3 lightingDir = normalize(-lightDirection);

            vec3 lightLook =  light.pos.xyz - worldPosition.xyz;

            float distance = max(length(lightLook), 0.001);

            float d2 = max(distance*distance, 0.01);

            lightLook /= distance;

            vec3 H = normalize(lightLook + viewPosToWorldPos);

            float diffuse = max(dot(normal.xyz, lightLook), 0.0);

            float cosTheta = dot(lightLook, lightingDir);

            float spotFactor = smoothstep(light.ancillary.w, light.ancillary.z, cosTheta);

            float specFactor = max(dot(H, normal), 0.0);

            specFactor = (diffuse > 0.0) ? pow(specFactor, shininess) : 0.0;

            float shadowColor = ShadowContribution(uvec2(lightViewIndex, 0), normal, lightingDir);

            DiffuseColor += lightColor * diffuse * diffuseIntensity * spotFactor * (1.0/d2) * shadowColor;

            SpecularColor += lightColor * specularBase * specFactor * specularIntensity * spotFactor * (1.0/d2) * shadowColor;
        }
    }

    return vec4(AmbientColor + (color.xyz * DiffuseColor + SpecularColor), color.w);
}

struct PixelColorData
{
    vec3 diffuse;
    vec3 specular;
    vec3 normal;
    vec3 emissive;
    float shininess;
};

PixelColorData ZeroColorData()
{
    PixelColorData data;

    data.diffuse = vec3(0.0);
    data.specular = vec3(0.0);
    data.normal = vec3(0.0);
    data.emissive = vec3(0.0);
    data.shininess = 0.0; 

    return data;
}

float GetPixelBlendWeight(BlendDetails details, uint textureIndex)
{
    float ret = 1.0; 

    switch(details.type)
    {
        case ConstantAlpha:
            ret = uintBitsToFloat(details.alphaMapHandleBlendFactor);
            break;
        case BlendMap:
            ret = texture(sampler2D(Textures[details.alphaMapHandleBlendFactor], samplerLinear), texCoords[0]).r;
            break;
        default:
            break;
    }

    return ret;
}

void main() 
{
    uint materialStart = rends.renderables[renderableIndex].materialStart;

    uint blendStart = rends.renderables[renderableIndex].blendLayersStart;

    uint materialCount = rends.renderables[renderableIndex].materialCount;

    mat3 normalMatrix = mat3(rends.renderables[renderableIndex].adjointMatrix);

    uint textureIndex = 0;

    PixelColorData accumulatedColorData = ZeroColorData();

    bool lightAffected = false;

    for (uint i = 0; i<materialCount; i++)
    {
        uint materialIndex = uint(texelFetch(globalMaterialIndices, int(materialStart + i)).r);

        uint blendIndex = uint(texelFetch(globalBlendIndices, int(blendStart + i)).r);

        Material mat = mats.matsData[materialIndex];

        BlendDetails details = blends.details[blendIndex];

        PixelColorData colorData = ZeroColorData();

        float w =  GetPixelBlendWeight(details, textureIndex);

        colorData.diffuse = mat.albedoColor.xyz;
        colorData.specular = mat.specular.xyz;
        colorData.emissive = mat.emissive.xyz;
        colorData.shininess = mat.shininess;
        
        if ((mat.materialFlags & ALBEDOMAPPED) == ALBEDOMAPPED)
        {
           colorData.diffuse *= texture(sampler2D(Textures[mat.textureHandles.x], samplerLinear), texCoords[textureIndex++]).rgb;
        }
    
        if ((mat.materialFlags & WORLDNORMALMAPPED) == WORLDNORMALMAPPED)
        {
            vec3 normal = texture(sampler2D(Textures[mat.textureHandles.y], samplerLinear), texCoords[textureIndex++]).rgb;

            colorData.normal = normalMatrix * (2.0 * normal.xyz - 1.0);

            lightAffected = true;
        } 
        else if ((mat.materialFlags & VERTEXNORMAL) == VERTEXNORMAL)
        {
            colorData.normal = inNormal;

            lightAffected = true;
        }
        else if ((mat.materialFlags & TANGENTNORMALMAPPED) == TANGENTNORMALMAPPED)
        {
            vec3 N = normalize(inNormal);
            vec3 T = normalize(inTangent.xyz);

            T = normalize(T - N * dot(N, T));
            vec3 B = inTangent.w * cross(N, T);

            mat3 TBN = mat3(T, B, N);

            vec3 normal = texture(sampler2D(Textures[mat.textureHandles.y], samplerLinear), texCoords[textureIndex++]).rgb;

            colorData.normal = TBN * (2.0 * normal.xyz - 1.0);

            lightAffected = true;
        }

        accumulatedColorData.diffuse += (colorData.diffuse * w);
        accumulatedColorData.normal += (colorData.normal * w);
        accumulatedColorData.specular += (colorData.specular * w);
        accumulatedColorData.emissive += (colorData.emissive * w);
        accumulatedColorData.shininess += (colorData.shininess * w);
    }

    vec4 albedoColor = vec4(accumulatedColorData.diffuse, 1.0);

    if (lightAffected)
    {
        albedoColor = DoLights(
            normalize(accumulatedColorData.normal), 
            albedoColor, 
            renderableIndex, 
            accumulatedColorData.specular, accumulatedColorData.shininess
        );
    }

    outColor = vertColor * (albedoColor + vec4(accumulatedColorData.emissive, 0.0));
}