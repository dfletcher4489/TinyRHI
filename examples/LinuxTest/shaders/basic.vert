#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 inTexCoords;

layout(location = 0) out vec2 outTexCoords;

layout(set = 0, binding = 0) uniform GlobalContext 
{
    mat4 view;
    mat4 proj;
    mat4 world;
} gs;

void main()
{
    gl_Position = gs.proj * gs.view * gs.world * position;

    outTexCoords = inTexCoords.xy;
}