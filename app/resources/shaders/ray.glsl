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

// SSBO STRUCUTRES AND SSBOs
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

// camera uniforms
uniform vec3 u_camera_position;
uniform vec3 u_camera_front;
uniform vec3 u_camera_up;
uniform vec3 u_camera_right;
uniform vec3 u_fov_tan;
uniform float u_aspect_ratio;

out vec4 FragColor;

struct Ray{
    vec3 origin;
    vec3 dir;
    vec3 inv_dir;
};

bool triangle_intersection(Ray ray){

}

bool aabb_intersection(Ray ray, vec3 min_bound, vec3 max_bound){

}


bool traverse_blas(Ray ray, uint root_index){
    // RECURSION WON'T WORK SO WE HAVE TO FAKE IT
    uint stack[32];
    stack[0] = root_index;
    int stack_ptr = 1;
    while(stack_ptr > 0){
        uint node_index = stack_ptr[--stack_ptr];
        Tlas node = tlas_tree[node_index];

        vec3 min_bound = node.min_bound;
        vec3 max_bound = node.max_bound;
        bool hit_box = aabb_intersection(ray, min_bound, max_bound);
        if(!hit_box){
            continue;
        }
        if(node.instance_count != 0){

        }else{
            stack[stack_ptr++] = node.left_child;
            stack[stack_ptr++] = node.right_child;
        }
    }

    return false;
}

bool traverse_tlas(Ray ray, uint root_index){
    // RECURSION WON'T WORK SO WE HAVE TO FAKE IT
    uint stack[32];
    stack[0] = root_index;
    int stack_ptr = 1;
    while(stack_ptr > 0){
        // get node from to of the stack
        uint node_index = stack_ptr[--stack_ptr];
        Tlas node = tlas_tree[node_index];

        vec3 min_bound = node.min_bound;
        vec3 max_bound = node.max_bound;
        bool hit_box = aabb_intersection(ray, min_bound, max_bound);
        // no hit on the box, go back
        if(!hit_box){
            continue;
        }
        // we hit somethiing
        // is it a leaf (model) ? 
        // or an inner node (bounding box) ?
        if(node.instance_count != 0){
            Ray local;
            uint model_root_index = tlas_tree[stack[stack_ptr]].first_instace;
            bool hit_model = traverse_blas(local, model_root_index);
            if(hit_model){
                return true;
            }
        }else{
            stack[stack_ptr++] = node.left_child;
            stack[stack_ptr++] = node.right_child;
        }
    }
    return false;
}

void main(){
    Ray ray;
    if(traverse_tlas(ray, 0)){

    }else{
        FragColor = vec4(0.1, 0.1, 0.1, 1.0);    
    }

}
