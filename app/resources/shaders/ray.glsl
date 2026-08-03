//#shader vertex
#version 430 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 v_uv;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}

//#shader fragment
#version 430 core

struct TlasNode{
    // 12 + 4 + 12 + 4 + 4 + 4 + 4 + 4 = 32 + 16 = 48 bytes good
    vec3 min_bound;
    float pad0;
    vec3 max_bound;
    float pad1;
    uint left_child;
    uint right_child;
    uint first_instace;
    uint instance_count;
};

struct GPUInstance{
    // 64 + 64 + 4 + 4 + 4 + 4 = 128 + 16 = 144 bytes good
    mat4 world_to_local;
    mat4 local_to_world;
    uint blas_root_index;
    uint material_index;
    uint pad0;
    uint pad1;
};

struct BlasNode{
    // 12 + 4 + 12 + 4 + 4 + 4 + 4 + 4 = 32 + 16 = 48 bytes good
    vec3 min_bound;
    float pad0;
    vec3 max_bound;
    float pad1;
    uint left_child;
    uint right_child;
    uint first_primitive;
    uint primitive_count;
};

struct GPUPrimitive{
    // 48 * 5 = 16 * 15 = something bytes good
    vec4 v0, v1, v2;
    vec4 n0, n1, n2;
    vec4 uv0, uv1, uv2;
    vec4 t0, t1, t2;
    vec4 b0, b1, b2;
};

layout(std430, binding = 0) readonly buffer BlasTreeBuffer{
    BlasNode blas_tree[];
};

layout(std430, binding = 1) readonly buffer PrimitivesBuffer{
    GPUPrimitive primitives[];
};

layout(std430, binding = 2) readonly buffer TlasTreeBuffer{
    TlasNode tlas_tree[];
};

layout(std430, binding = 3) readonly buffer InstancesBuffer{
    GPUInstance instances[];
};

out vec4 FragColor;

void main(){
    FragColor = vec4(0.0, 0.5, 0.5, 1.0);    
}
