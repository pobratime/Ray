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

struct TlasNode {
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
    mat4 world_to_local;
    mat4 local_to_world;
    uint blas_root_index;
    uint material_index;
    uint pad0;
    uint pad1;
};

struct BlasNode {
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
    vec4 v0, v1, v2;
    vec4 n0, n1, n2;
    vec4 uv0, uv1, uv2;
    vec4 t0, t1, t2;
    vec4 b0, b1, b2;
    vec4 t_idx;
};

struct GPULightSource {
    vec4 position;
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

layout(std430, binding = 4) readonly buffer LightsBuffer {
    GPULightSource lights[];
};

uniform sampler2DArray u_Textures;

// camera uniforms
uniform vec3 u_camera_position;
uniform vec3 u_camera_front;
uniform vec3 u_camera_up;
uniform vec3 u_camera_right;
uniform float u_fov_tan;
uniform float u_aspect_ratio;

uniform int u_light_count;

// RenderSettings
uniform int u_light_samples;
uniform int u_reflection_count;
uniform float u_min_reflection;
uniform float u_light_power;
uniform float u_ambient;
uniform bool u_use_textures;
uniform int u_ao_samples;
uniform float u_ao_radius;

// Radius of the emitter used for soft shadows. Roughly the size of the bulb.
const float LIGHT_RADIUS = 0.02;

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

HitData primitive_hit;

vec4 sample_tex(int index, vec2 uv) {
    return textureLod(u_Textures, vec3(uv, float(index)), 0.0);
}

// Möller–Trumbore intersection
bool triangle_intersection(Ray ray, GPUPrimitive triangle, uint primitive_index, uint instance_index) {
    const float EPS = 1e-12;
    const float T_MIN = 1e-4;
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

    if (t > T_MIN && t < primitive_hit.t) {
        primitive_hit.t = t;
        primitive_hit.primitive_index = primitive_index;
        primitive_hit.instance_index = instance_index;
        primitive_hit.uv = vec2(u, v);
        primitive_hit.hit = true;
        return true;
    }

    return false;
}

// Slab algorithm AABB intersection
bool aabb_intersection(Ray ray, vec3 min_bound, vec3 max_bound) {
    vec3 t0 = (min_bound - ray.origin) * ray.inv_dir;
    vec3 t1 = (max_bound - ray.origin) * ray.inv_dir;

    vec3 tmin_vec = min(t0, t1);
    vec3 tmax_vec = max(t0, t1);

    float t_entry = max(max(tmin_vec.x, tmin_vec.y), tmin_vec.z);
    float t_exit = min(min(tmax_vec.x, tmax_vec.y), tmax_vec.z);

    t_entry = max(t_entry, 0.0);

    return (t_entry <= t_exit) && (t_entry < primitive_hit.t);
}

bool traverse_blas(Ray ray, uint root_index, uint instance_index, bool any_hit) {
    bool hit = false;
    uint stack[13];
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
                uint primitive_index = node.first_primitive + i;
                GPUPrimitive triangle = primitives[primitive_index];
                if (triangle_intersection(ray, triangle, primitive_index, instance_index)) {
                    hit = true;
                    if (any_hit && int(triangle.t_idx.z) == -1) {
                        return true;
                    }
                }
            }
        } else {
            stack[stack_ptr++] = node.left_child;
            stack[stack_ptr++] = node.right_child;
        }
    }
    return hit;
}

bool traverse_tlas(Ray ray, uint root_index, bool any_hit) {
    bool hit = false;
    uint stack[4];
    stack[0] = root_index;
    int stack_ptr = 1;
    while (stack_ptr > 0) {
        uint node_index = stack[--stack_ptr];
        TlasNode node = tlas_tree[node_index];

        vec3 min_bound = node.min_bound;
        vec3 max_bound = node.max_bound;
        bool hit_box = aabb_intersection(ray, min_bound, max_bound);
        if (!hit_box) {
            continue;
        }
        if (node.instance_count != 0) {
            uint instance_index = node.first_instance;
            GPUInstance instance = instances[instance_index];

            Ray local;
            local.origin = vec3(instance.world_to_local * vec4(ray.origin, 1.0));
            local.dir = vec3(instance.world_to_local * vec4(ray.dir, 0.0));
            local.inv_dir = 1.0 / local.dir;

            uint model_root_index = instance.blas_root_index;
            if (traverse_blas(local, model_root_index, instance_index, any_hit)) {
                hit = true;
                if(any_hit){
                    return true;
                }
            }
        } else {
            stack[stack_ptr++] = node.left_child;
            stack[stack_ptr++] = node.right_child;
        }
    }
    return hit;
}

vec3 hit_barycentric() {
    float u = primitive_hit.uv.x;
    float v = primitive_hit.uv.y;
    return vec3(1.0 - u - v, u, v);
}

vec2 interpolate_uv(GPUPrimitive tri, vec3 b) {
    return b.x * tri.uv0.xy + b.y * tri.uv1.xy + b.z * tri.uv2.xy;
}

vec3 hit_normal(GPUPrimitive tri, GPUInstance inst, vec3 b, vec2 uv, vec3 ray_dir) {
    vec3 n_local = b.x * tri.n0.xyz + b.y * tri.n1.xyz + b.z * tri.n2.xyz;

    int norm_idx = int(tri.t_idx.y);
    if (u_use_textures && norm_idx != -1) {
        vec3 t = normalize(b.x * tri.t0.xyz + b.y * tri.t1.xyz + b.z * tri.t2.xyz);
        vec3 bt = normalize(b.x * tri.b0.xyz + b.y * tri.b1.xyz + b.z * tri.b2.xyz);
        vec3 tex_n = sample_tex(norm_idx, uv).rgb * 2.0 - 1.0;
        n_local = mat3(t, bt, normalize(n_local)) * tex_n;
    }

    vec3 n = normalize(transpose(mat3(inst.world_to_local)) * n_local);
    return (dot(n, ray_dir) > 0.0) ? -n : n;
}

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

// okej
// ne vidim puno prostora za optimizaciju
vec3 shadow_ray(vec3 world_hit_pos, vec3 normal) {
    vec3 total_light = vec3(0.0);
    if(u_light_count == 0){
        return total_light;
    }
    for (int i = 0; i < u_light_count; i++) {
        vec3 accum = vec3(0.0);

        for (int s = 0; s < u_light_samples; s++) {
            vec3 target = lights[i].position.xyz;
            if (u_light_samples > 1) {
                vec2 seed = world_hit_pos.xy + world_hit_pos.z + float(s) * 17.3 + float(i) * 91.7;
                vec3 jitter = vec3(rand(seed), rand(seed + 5.1), rand(seed + 23.9)) * 2.0 - 1.0;
                target += jitter * LIGHT_RADIUS;
            }

            vec3 ray = target - world_hit_pos;
            float dist = length(ray);
            vec3 ray_dir = ray / dist;

            // trougao nije okrenut ka izvora svetla, ne proveravaj ga
            float n_dot_l = dot(normal, ray_dir);
            if (n_dot_l <= 0.0) {
                continue;
            }

            Ray sray;
            sray.origin = world_hit_pos + normal * 1e-3;
            sray.dir = ray_dir;
            sray.inv_dir = 1.0 / ray_dir;

            HitData old = primitive_hit;
            primitive_hit.hit = false;
            primitive_hit.t = dist;

            if (traverse_tlas(sray, 0, true)) {
                GPUPrimitive tri = primitives[primitive_hit.primitive_index];
                int emis_idx = int(tri.t_idx.z);
                if (emis_idx != -1) {
                    vec2 iuv = interpolate_uv(tri, hit_barycentric());
                    vec3 emissive_col = sample_tex(emis_idx, iuv).rgb;
                    if (dot(emissive_col, emissive_col) > 0.01) {
                        accum += emissive_col * n_dot_l * u_light_power / (1.0 + dist * dist);
                    }
                }
            }
            primitive_hit = old;
        }

        total_light += accum / float(u_light_samples);
    }
    return total_light;
}

float ambient_occlusion(vec3 world_hit_pos, vec3 normal) {
    if (u_ao_samples == 0) {
        return 1.0;
    }
    int blocked = 0;
    for (int i = 0; i < u_ao_samples; i++) {
        vec2 seed = world_hit_pos.xy + world_hit_pos.z + float(i) * 31.7;
        vec3 rnd = vec3(rand(seed), rand(seed.yx * 1.7), rand(seed * 3.3 + 11.0)) * 2.0 - 1.0;
        // normal + a random point in a cube lands somewhere in the hemisphere above the surface
        vec3 dir = normal + rnd;
        if (dot(dir, dir) < 1e-6) {
            dir = normal;
        }
        dir = normalize(dir);

        Ray aray;
        aray.origin = world_hit_pos + normal * 1e-3;
        aray.dir = dir;
        aray.inv_dir = 1.0 / dir;

        HitData old = primitive_hit;
        primitive_hit.hit = false;
        primitive_hit.t = u_ao_radius;
        if (traverse_tlas(aray, 0, true)) {
            blocked++;
        }
        primitive_hit = old;
    }
    return 1.0 - float(blocked) / float(u_ao_samples);
}

// nema puno filozofije
vec3 texture_primitive(GPUPrimitive tri, GPUInstance inst, vec3 world_hit_pos) {
    int diff_idx = int(tri.t_idx.x);
    if (!u_use_textures || diff_idx == -1) {
        return vec3(1.0);
    }
    vec3 bar = hit_barycentric();
    vec2 iuv = interpolate_uv(tri, bar);
    return sample_tex(diff_idx, iuv).rgb;
}

vec3 reflection_ray(vec3 pos, GPUPrimitive tri, GPUInstance inst, vec3 ray_dir) {
    if(u_reflection_count == 0){
        return vec3(0.0, 0.0, 0.0);
    }
    HitData old = primitive_hit;

    vec3 accum = vec3(0.0);
    vec3 throughput = vec3(1.0);

    GPUPrimitive cur_tri = tri;
    GPUInstance cur_inst = inst;
    vec3 cur_pos = pos;
    vec3 cur_dir = ray_dir;

    for (int bounce = 0; bounce < u_reflection_count; bounce++) {
        vec3 b = hit_barycentric();
        vec2 iuv = interpolate_uv(cur_tri, b);

        int arm_idx = int(cur_tri.t_idx.w);
        if (arm_idx == -1) {
            break;
        }

        vec4 arm_tex = sample_tex(arm_idx, iuv);
        float metalness = arm_tex.b;
        float roughness = arm_tex.g;
        if (metalness < u_min_reflection) {
            break;
        }

        vec3 n = hit_normal(cur_tri, cur_inst, b, iuv, cur_dir);
        vec3 mirror = reflect(cur_dir, n);
        vec3 scattered = mirror;
        if (roughness > 0.15) {
            vec2 seed = gl_FragCoord.xy + float(bounce) * 7.31;
            vec3 jitter = vec3(rand(seed), rand(seed.yx * 1.7), rand(seed * 3.3 + 11.0)) * 2.0 - 1.0;
            scattered = normalize(mirror + jitter * roughness * roughness);
            if (dot(scattered, n) < 0.0) scattered = mirror;
        }


        Ray rray;
        rray.dir = scattered;
        rray.origin = cur_pos + n * 1e-3;
        rray.inv_dir = 1.0 / rray.dir;

        throughput *= metalness * texture_primitive(cur_tri, cur_inst, cur_pos);

        primitive_hit.hit = false;
        primitive_hit.t = 1e30;

        if (!traverse_tlas(rray, 0, false)) {
            break;
        }

        vec3 p2 = rray.origin + rray.dir * primitive_hit.t;
        GPUPrimitive t2 = primitives[primitive_hit.primitive_index];
        GPUInstance i2 = instances[primitive_hit.instance_index];
        vec3 b2 = hit_barycentric();
        vec2 iuv2 = interpolate_uv(t2, b2);
        vec3 n2 = hit_normal(t2, i2, b2, iuv2, rray.dir);

        accum += throughput * texture_primitive(t2, i2, p2) * (vec3(u_ambient) + shadow_ray(p2, n2));
        cur_tri = t2;
        cur_inst = i2;
        cur_pos = p2;
        cur_dir = rray.dir;
    }

    primitive_hit = old;
    return accum;
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

    if (traverse_tlas(ray, 0, false)) {
        GPUInstance instance = instances[primitive_hit.instance_index];
        GPUPrimitive tri = primitives[primitive_hit.primitive_index];
        vec3 b = hit_barycentric();
        vec2 iuv = interpolate_uv(tri, b);

        int emis_idx = int(tri.t_idx.z);
        if (emis_idx != -1) {
            vec3 emissive_col = sample_tex(emis_idx, iuv).rgb;
            if (dot(emissive_col, emissive_col) > 0.01) {
                FragColor = vec4(emissive_col * 10.0, 1.0);
                return;
            }
        }

        vec3 world_hit_pos = ray.origin + ray.dir * primitive_hit.t;
        vec3 n = hit_normal(tri, instance, b, iuv, ray.dir);
        vec3 direct_light = shadow_ray(world_hit_pos, n);
        vec3 base_color = texture_primitive(tri, instance, world_hit_pos);
        float ao = ambient_occlusion(world_hit_pos, n);
        vec3 final_color = base_color * (vec3(u_ambient) * ao + direct_light);
        final_color += reflection_ray(world_hit_pos, tri, instance, ray.dir);
        FragColor = vec4(final_color, 1.0);
    } else {
        FragColor = vec4(0.08, 0.08, 0.12, 1.0);
    }
}
