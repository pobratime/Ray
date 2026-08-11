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

uniform sampler2D u_image;
uniform bool u_horizontal;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec4 FragColor;

void main() {
    vec2 texel = 1.0 / vec2(textureSize(u_image, 0));

    vec3 result = texture(u_image, v_uv).rgb * weights[0];
    for (int i = 1; i < 5; i++) {
        vec2 offset = u_horizontal ? vec2(texel.x * float(i), 0.0) : vec2(0.0, texel.y * float(i));
        result += texture(u_image, v_uv + offset).rgb * weights[i];
        result += texture(u_image, v_uv - offset).rgb * weights[i];
    }
    FragColor = vec4(result, 1.0);
}
