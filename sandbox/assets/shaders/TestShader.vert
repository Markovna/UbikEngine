#version 420 core

layout(location = 0) in vec3 _POSITION;
layout(location = 1) in vec2 _TEXCOORD0;

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform Model {
    mat4 model;
};

layout(location = 0) out vec2 vTexCoord;

void main() {
    gl_Position = projection * view * model * vec4(_POSITION, 1.0);
    vTexCoord = _TEXCOORD0;
}