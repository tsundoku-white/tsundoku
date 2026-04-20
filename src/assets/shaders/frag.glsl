#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;

void main()
{
    // Simple directional light so the model isn't flat-shaded
    vec3  light_dir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse   = max(dot(normalize(frag_normal), light_dir), 0.0);
    float ambient   = 0.2;

    vec3 lit = frag_color * (ambient + diffuse);
    out_color = vec4(lit, 1.0);
}
