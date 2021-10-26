#define code(__code) #__code "\0"
{
  .vertex_shader = code(
    in vec3 _POSITION;
    in vec2 _TEXCOORD0;

    out vec2 vTexCoord;

    layout(std140) uniform CameraParams {
      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;
    };

    void main()
    {
        gl_Position = projection * view * model * vec4(_POSITION, 1.0);
        vTexCoord = _TEXCOORD0;
    }),
  .fragment_shader = code(

    in vec2 vTexCoord;

    out vec4 FragColor;

    //uniform vec4 mainColor;
    uniform sampler2D Texture1;
    uniform sampler2D Texture2;

    void main()
    {
      FragColor = mix(texture(Texture1, vTexCoord), texture(Texture2, vTexCoord), 0.7);
    }),
  .metadata = code(
    Texture1 1 0
    Texture2 1 1
    CameraParams 0 2
  )
}
#undef code