#version 450

layout(location = 0) out vec4 color;

void main()
{
    vec2 positions[3] = vec2[](
        vec2(-1.0, 1.0),
        vec2(0.0, -1.0), 
        vec2(1.0, 1.0)
    );

    vec4 colors[3] = vec4[](
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    color = colors[gl_VertexIndex];
}