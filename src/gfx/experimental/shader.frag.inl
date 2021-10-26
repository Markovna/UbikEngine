#version 410 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform vec4 mainColor;
uniform sampler2D Texture1;
uniform sampler2D Texture2;

//#define layout_binding(__binding) layout(binding = __binding)
//
//layout_binding(1)

uniform sampler2D texSampler;

//layout(std140) uniform MainBlock
//{
//    vec3 data;
//};

void main()
{
    FragColor = mainColor * mix(texture(Texture1, vTexCoord), texture(Texture2, vTexCoord), 0.7);
}