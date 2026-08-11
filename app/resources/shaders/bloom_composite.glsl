//#shader vertex
#version 460 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 v_uv;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}

//#shader fragment
#version 460 core

in vec2 v_uv;

uniform sampler2D u_scene;
uniform sampler2D u_bloom;
uniform float u_intensity;

out vec4 FragColor;

void main() {
    vec3 color = texture(u_scene, v_uv).rgb;
    color += texture(u_bloom, v_uv).rgb * u_intensity;
    FragColor = vec4(color, 1.0);
}
