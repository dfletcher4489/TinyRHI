#version 450

layout(location = 0) out vec4 color;

void main()
{
    vec2 positions[4] = vec2[](
        vec2(-1.0, -1.0), // bottom-left
        vec2( 1.0, -1.0), // bottom-right
        vec2(-1.0,  1.0), // top-left
        vec2( 1.0,  1.0)  // top-right
    );

    vec4 colors[4] = vec4[](
        vec4(1.0, 1.0, 1.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0),
        vec4(1.0, 1.0, 1.0, 1.0)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.5, 1.0);

    color = colors[gl_VertexIndex];
}