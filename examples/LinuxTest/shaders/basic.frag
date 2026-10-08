#version 450

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform texture2D Texture;
layout(set = 0, binding = 2) uniform sampler samplerLinear;

void main() 
{
    outColor = texture(sampler2D(Texture, samplerLinear), texCoords); 
}