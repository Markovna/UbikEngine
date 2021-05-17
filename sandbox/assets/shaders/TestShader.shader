#type vertex
#version 330 core

#binding Position
in vec3 aPos;

#binding TexCoord0
in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vTexCoord = aTexCoord;
}

#type fragment
#version 330 core

//in vec4 vColor;
in vec2 vTexCoord;

out vec4 FragColor;

uniform vec4 mainColor;
uniform sampler2D Texture1;
uniform sampler2D Texture2;

void main()
{
    FragColor = mainColor * mix(texture(Texture1, vTexCoord), texture(Texture2, vTexCoord), 0.7);
}