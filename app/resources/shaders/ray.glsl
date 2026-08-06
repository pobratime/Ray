//#shader vertex
#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 v_uv;

void main() {
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}

//#shader fragment
#version 430 core

in vec2 v_uv;

// SSBO STRUCUTRES AND SSBOs
struct TlasNode {
// 12 + 4 + 12 + 4 + 4 + 4 + 4 + 4 = 32 + 16 = 48 bytes good
    vec3 min_bound;
    float pad0;
    vec3 max_bound;
    float pad1;
    uint left_child;
    uint right_child;
    uint first_instance;
    uint instance_count;
};

struct GPUInstance {
// 64 + 64 + 4 + 4 + 4 + 4 = 128 + 16 = 144 bytes good
    mat4 world_to_local;
    mat4 local_to_world;
    uint blas_root_index;
    uint material_index;
    uint pad0;
    uint pad1;
};

struct BlasNode {
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

struct GPUPrimitive {
// 48 * 5 = 16 * 15 = something bytes good
    vec4 v0, v1, v2;
    vec4 n0, n1, n2;
    vec4 uv0, uv1, uv2;
    vec4 t0, t1, t2;
    vec4 b0, b1, b2;
    vec4 t_idx;
};

layout (std430, binding = 0) readonly buffer BlasTreeBuffer {
    BlasNode blas_tree[];
};

layout (std430, binding = 1) readonly buffer PrimitivesBuffer {
    GPUPrimitive primitives[];
};

layout (std430, binding = 2) readonly buffer TlasTreeBuffer {
    TlasNode tlas_tree[];
};

layout (std430, binding = 3) readonly buffer InstancesBuffer {
    GPUInstance instances[];
};

//  Diffuse
//  Specular
//  Normal
//  Height

uniform sampler2D u_Textures[16];

// camera uniforms
uniform vec3 u_camera_position;
uniform vec3 u_camera_front;
uniform vec3 u_camera_up;
uniform vec3 u_camera_right;
uniform float u_fov_tan;
uniform float u_aspect_ratio;

out vec4 FragColor;

struct Ray {
    vec3 origin;
    vec3 dir;
    vec3 inv_dir;
};

struct HitData {
    float t;
    uint primitive_index;
    uint instance_index;
    vec2 uv;
    bool hit;
};

// global hit data
HitData primitive_hit;

// TODO -> add switch option
bool triangle_intersection2(Ray ray, GPUPrimitive triangle, uint primitive_index, uint instance_index) {
    vec3 v0 = triangle.v0.xyz;
    vec3 v1 = triangle.v1.xyz;
    vec3 v2 = triangle.v2.xyz;

    vec3 u = v0 - ray.origin;
    vec3 v = v1 - ray.origin;
    vec3 w = v2 - ray.origin;

    float sign1 = dot(cross(u, v), ray.dir);
    float sign2 = dot(cross(v, w), ray.dir);
    float sign3 = dot(cross(w, u), ray.dir);

    bool all_pos = (sign1 >= 0.0) && (sign2 >= 0.0) && (sign3 >= 0.0);
    bool all_neg = (sign1 <= 0.0) && (sign2 <= 0.0) && (sign3 <= 0.0);

    if (!all_pos && !all_neg) {
        return false;
    }

    // M = ray.origin + t * ray.dir
    float sum = sign1 + sign2 + sign3;
    if (abs(sum) < 1e-7) {
        // degenirsan trougao
        // a i izbegavamo deljenje nulom
        return false;
    }

    float t = (dot(cross(u, v), w)) / sum;

    if (!primitive_hit.hit || (t > 1e-6 && t < primitive_hit.t)) {
        primitive_hit.t = t;
        primitive_hit.primitive_index = primitive_index;
        primitive_hit.instance_index = instance_index;
        float inv_sum = 1.0 / sum;
        primitive_hit.uv = vec2(sign2 * inv_sum, sign3 * inv_sum);
        primitive_hit.hit = true;
        return true;
    }

    return false;
}

// Moller–Trumbore
bool triangle_intersection(Ray ray, GPUPrimitive triangle, uint primitive_index, uint instance_index) {
    const float EPS = 1e-12;
    vec3 v0 = triangle.v0.xyz;
    vec3 v1 = triangle.v1.xyz;
    vec3 v2 = triangle.v2.xyz;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(ray.dir, edge2);
    float a = dot(edge1, h);

    if (abs(a) < EPS) return false;

    float f = 1.0 / a;
    vec3 s = ray.origin - v0;
    float u = f * dot(s, h);

    if (u < -EPS || u > 1.0 + EPS) return false;

    vec3 q = cross(s, edge1);
    float v = f * dot(ray.dir, q);

    if (v < -EPS || (u + v) > 1.0 + EPS) return false;

    float t = f * dot(edge2, q);

    if (t > EPS && (!primitive_hit.hit || t < primitive_hit.t)) {
        primitive_hit.t = t;
        primitive_hit.primitive_index = primitive_index;
        primitive_hit.instance_index = instance_index;
        primitive_hit.uv = vec2(u, v);
        primitive_hit.hit = true;
        return true;
    }

    return false;
}

bool aabb_intersection(Ray ray, vec3 min_bound, vec3 max_bound) {
    vec3 t0 = (min_bound - ray.origin) * ray.inv_dir;
    vec3 t1 = (max_bound - ray.origin) * ray.inv_dir;

    vec3 tmin_vec = min(t0, t1);
    vec3 tmax_vec = max(t0, t1);

    float t_entry = max(max(tmin_vec.x, tmin_vec.y), tmin_vec.z);
    float t_exit = min(min(tmax_vec.x, tmax_vec.y), tmax_vec.z);

    t_entry = max(t_entry, 0.0);

    return (t_entry <= t_exit) && (!primitive_hit.hit || t_entry < primitive_hit.t);
}


bool traverse_blas(Ray ray, uint root_index, uint instance_index) {
    bool hit = false;
    // RECURSION WON'T WORK SO WE HAVE TO FAKE IT
    uint stack[128];
    stack[0] = root_index;
    int stack_ptr = 1;
    while (stack_ptr > 0) {
        uint node_index = stack[--stack_ptr];
        BlasNode node = blas_tree[node_index];

        vec3 min_bound = node.min_bound;
        vec3 max_bound = node.max_bound;
        bool hit_box = aabb_intersection(ray, min_bound, max_bound);
        if (!hit_box) {
            continue;
        }
        if (node.primitive_count != 0) {
            for (uint i = 0; i < node.primitive_count; i++) {
                // check all primitives individually
                uint primitive_index = node.first_primitive + i;
                GPUPrimitive triangle = primitives[primitive_index];
                if (triangle_intersection(ray, triangle, primitive_index, instance_index)) {
                    hit = true;
                }
            }
        } else {
            stack[stack_ptr++] = node.left_child;
            stack[stack_ptr++] = node.right_child;
        }
    }
    return hit;
}

bool traverse_tlas(Ray ray, uint root_index) {
    // RECURSION WON'T WORK SO WE HAVE TO FAKE IT
    bool hit = false;
    uint stack[128];
    stack[0] = root_index;
    int stack_ptr = 1;
    while (stack_ptr > 0) {
        // get node from to of the stack
        uint node_index = stack[--stack_ptr];
        TlasNode node = tlas_tree[node_index];

        vec3 min_bound = node.min_bound;
        vec3 max_bound = node.max_bound;
        bool hit_box = aabb_intersection(ray, min_bound, max_bound);
        // no hit on the box, go back
        if (!hit_box) {
            continue;
        }
        // we hit something
        // is it a leaf (model) ?
        // or an inner node (bounding box) ?
        if (node.instance_count != 0) {
            // RAY WORLD TO RAY LOCAL
            // DON'T FORGET LIIKE LAST TIME
            // no need for a for-loop since we have one model per leaf
            uint instance_index = node.first_instance;
            GPUInstance instance = instances[instance_index];

            Ray local;
            local.origin = vec3(instance.world_to_local * vec4(ray.origin, 1.0));
            local.dir = vec3(instance.world_to_local * vec4(ray.dir, 0.0));
            local.inv_dir = 1.0 / local.dir;

            uint model_root_index = instance.blas_root_index;
            if (traverse_blas(local, model_root_index, instance_index)) {
                hit = true;
            }
        } else {
            stack[stack_ptr++] = node.left_child;
            stack[stack_ptr++] = node.right_child;
        }
    }
    return hit;
}

vec3 texture_primitive(GPUPrimitive tri, GPUInstance inst) {
    int diff_idx = int(tri.t_idx.x);
    int spec_idx = int(tri.t_idx.y);
    int norm_idx = int(tri.t_idx.z);
    int high_idx = int(tri.t_idx.w);

    float u = primitive_hit.uv.x;
    float v = primitive_hit.uv.y;
    float w = 1.0f - u - v;

    vec2 interpolated_uv = w * tri.uv0.xy + u * tri.uv1.xy + v * tri.uv2.xy;

    // textureLod bff <3 <3
    vec3 diff_color = (diff_idx != -1) ? textureLod(u_Textures[diff_idx], interpolated_uv, 0.0).rgb : vec3(1.0f);
    vec3 norm_color = (norm_idx != -1) ? textureLod(u_Textures[norm_idx], interpolated_uv, 0.0).rgb : vec3(0.5, 0.5, 1.0);
    float spec_color = (spec_idx != -1) ? textureLod(u_Textures[spec_idx], interpolated_uv, 0.0).r : 0.2f;

    return diff_color;
}

vec3 color_primitive(GPUPrimitive tri, GPUInstance inst) {
    vec3 edge1 = tri.v1.xyz - tri.v0.xyz;
    vec3 edge2 = tri.v2.xyz - tri.v0.xyz;
    vec3 local_normal = normalize(cross(edge1, edge2));

    mat3 normal_matrix = transpose(mat3(inst.world_to_local));
    vec3 world_normal = normalize(normal_matrix * local_normal);

    vec3 normal_color = world_normal * 0.5 + 0.5;

    return normal_color;
}

void main() {
    primitive_hit.hit = false;
    primitive_hit.t = 1e30;

    vec2 screen_uv = v_uv * 2.0 - 1.0;
    vec3 ray_dir = normalize(
            u_camera_front +
            u_camera_right * screen_uv.x * u_fov_tan * u_aspect_ratio +
            u_camera_up * screen_uv.y * u_fov_tan
    );

    Ray ray;
    ray.origin = u_camera_position;
    ray.dir = ray_dir;
    ray.inv_dir = 1.0 / ray.dir;

    if (traverse_tlas(ray, 0)) {
        GPUInstance instance = instances[primitive_hit.instance_index];
        GPUPrimitive tri = primitives[primitive_hit.primitive_index];

        int diffuse_tex_idx = int(tri.t_idx.x);
        if (diffuse_tex_idx != -1) {
            FragColor = vec4(texture_primitive(tri, instance), 1.0f);
        } else {
            FragColor = vec4(color_primitive(tri, instance), 1.0f);
        }
    } else {
        FragColor = vec4(0.08, 0.08, 0.12, 1.0);
    }
}
