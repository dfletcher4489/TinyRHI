#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 inTexCoords;

layout(location = 0) out vec2 outTexCoords;

layout(set = 0, binding = 0) uniform GlobalContext 
{
    mat4 view;
    mat4 proj;
} gs;


void main()
{
    vec2 positions[3] = vec2[](
        vec2(-1.0, 1.0),
        vec2(0.0, -1.0), 
        vec2(1.0, 1.0)
    );

    vec4 colors[6] = vec4[](
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(1.0, 1.0, 0.0, 1.0),
        vec4(0.0, 1.0, 1.0, 1.0),
        vec4(1.0, 0.0, 1.0, 1.0)
    );

    gl_Position = gs.proj * gs.view * position;

    outTexCoords = inTexCoords.xy;
}