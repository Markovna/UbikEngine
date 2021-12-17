#version 420 core

layout(location = 0) in vec2 _POSITION;
layout(location = 1) in vec2 _TEXCOORD0;
layout(location = 2) in vec4 _COLOR0;

layout(std140, binding = 0) uniform Camera {
    mat4 projection;
};

layout(location = 0) out vec2 Frag_UV;
layout(location = 1) out vec4 Frag_Color;

void main()
{
    Frag_UV = _TEXCOORD0;
    Frag_Color = _COLOR0;
//    vec4 pos = projection * vec4(_POSITION.xy, 0, 1);
//    gl_Position = vec4(pos.x, -pos.y, 0,1);
    gl_Position = projection * vec4(_POSITION.xy, 0, 1);
}