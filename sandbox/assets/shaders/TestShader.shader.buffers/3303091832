#version 420 core

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D Texture1;
layout(binding = 1) uniform sampler2D Texture2;

void main()
{
    FragColor = mix(texture(Texture1, vTexCoord), texture(Texture2, vTexCoord), 0.7);
}