#type vertex
#version 330 core

#binding Position
in vec2 Position;

#binding TexCoord0
in vec2 UV;

#binding Color0
in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    Frag_UV = UV;
    Frag_Color = Color;
    vec4 pos = projection * vec4(Position.xy, 0,1);
    gl_Position = vec4(pos.x, -pos.y, 0,1);
}

#type fragment
#version 330 core

in vec2 Frag_UV;
in vec4 Frag_Color;

out vec4 Out_Color;

uniform sampler2D Texture;

void main()
{
    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
}