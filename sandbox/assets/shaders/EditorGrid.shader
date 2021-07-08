#type vertex
#version 330 core

#binding Position
in vec3 aPos;

#binding TexCoord0
in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}

#type fragment
#version 330 core

in vec2 vTexCoord;

out vec4 outColor;

uniform float log10_inv = 1 / log(10);
uniform float cell_size = 0.0001f; // minimum size of a grid cell in world units that will be visualized.
uniform vec4 thin_color = vec4(0.5f, 0.5f, 0.5f, 1.0);
uniform vec4 thick_color = vec4(0.6, 0.6, 0.6, 1.0);
uniform float grid_size = 0.5f;

void main()
{
    vec2 uv = vTexCoord;

    // Find screen-space derivates of grid space. [1]
    vec2 dudv = vec2(
        length(vec2(dFdx(uv.x), dFdy(uv.x))),
        length(vec2(dFdx(uv.y), dFdy(uv.y)))
    );

    // Define minimum number of pixels between cell lines before LOD switch should occur.
    const float min_pixels_between_cells = 4.f;

    // Calc lod-level [2].
    float lod_level = max(0, log10_inv * log(length(dudv) * min_pixels_between_cells / cell_size) + 1.0);
    float lod_fade = fract(lod_level);

    // Calc cell sizes for lod0, lod1 and lod2.
    float lod0_cs = cell_size * pow(10, floor(lod_level));
    float lod1_cs = lod0_cs * 10.f;
    float lod2_cs = lod1_cs * 10.f;

    // Allow each anti-aliased line to cover up to 2 pixels.
    dudv *= 2;
    uv += dudv * 0.5f;

    // Calculate unsigned distance to cell line center for each lod [3]
    vec2 lod0_cross_a = 1.f - abs(clamp(mod(uv, lod0_cs) / dudv, 0.0, 1.0) * 2 - 1.f);
    // Pick max of x,y to get a coverage alpha value for lod
    float lod0_a = max(lod0_cross_a.x, lod0_cross_a.y);

    vec2 lod1_cross_a = 1.f - abs(clamp(mod(uv, lod1_cs) / dudv, 0.0, 1.0) * 2 - 1.f);
    float lod1_a = max(lod1_cross_a.x, lod1_cross_a.y);

    vec2 lod2_cross_a = 1.f - abs(clamp(mod(uv, lod2_cs) / dudv, 0.0, 1.0) * 2 - 1.f);
    float lod2_a = max(lod2_cross_a.x, lod2_cross_a.y);

    // Blend between falloff colors to handle LOD transition [4]
    vec4 c = lod2_a > 0 ? thick_color : lod1_a > 0 ? mix(thick_color, thin_color, lod_fade) : thin_color;

    // Calculate opacity falloff based on distance to grid extents. [5]
    float op = 1.f - clamp(length(uv) / grid_size, 0.0, 1.0);

    // Blend between LOD level alphas and scale with opacity falloff. [6]
    c.a *= (lod2_a > 0 ? lod2_a : lod1_a > 0 ? lod1_a : (lod0_a  * (1.0 - lod_fade))) * op;

    outColor = c;
}