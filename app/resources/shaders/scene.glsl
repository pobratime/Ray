//#shader vertex
#version 430 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 v_uv;

void main(){
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0f, 1.0f);
}

//#shader fragment
#version 430 core
in vec2 v_uv;
out vec4 FragColor;

uniform samplerBuffer u_nodes;
uniform samplerBuffer u_primitives;

uniform vec3 u_camera_pos;
uniform vec3 u_camera_front;
uniform vec3 u_camera_up;
uniform vec3 u_camera_right;
uniform float u_fov_tan;
uniform float u_aspect_ratio;

struct Node{
    vec3 min_bound;
    vec3 max_bound;
    uint left_child;
    uint right_child;
    uint first_primitive;
    uint primitive_count;
};

struct Primitive{
    vec3 v0, v1, v2;
    vec3 n0, n1, n2;
};

Node fetch_node(int index){
    int base = index * 3;
    vec4 t0 = texelFetch(u_nodes, base + 0);
    vec4 t1 = texelFetch(u_nodes, base + 1);
    vec4 t2 = texelFetch(u_nodes, base + 2);

    Node node;
    node.min_bound = t0.xyz;
    node.max_bound = t1.xyz;
    node.left_child = floatBitsToUint(t2.x);
    node.right_child = floatBitsToUint(t2.y);
    node.first_primitive = floatBitsToUint(t2.z);
    node.primitive_count = floatBitsToUint(t2.w);
    return node;
}

Primitive fetch_primitive(int index){
    int base = index * 6;
    Primitive prim;
    prim.v0 = texelFetch(u_primitives, base + 0).xyz;
    prim.v1 = texelFetch(u_primitives, base + 1).xyz;
    prim.v2 = texelFetch(u_primitives, base + 2).xyz;
    prim.n0 = texelFetch(u_primitives, base + 3).xyz;
    prim.n1 = texelFetch(u_primitives, base + 4).xyz;
    prim.n2 = texelFetch(u_primitives, base + 5).xyz;
    return prim;
}

struct Ray{
    vec3 origin;
    vec3 dir;
    vec3 dir_inv;
};

bool triangle_intersection(const in Ray r, const in Primitive prim, out vec3 M, out float hit_t) {
//    TROUGAO
    vec3 u = prim.v1 - prim.v0;
    vec3 w = prim.v2 - prim.v0;
    vec3 v = prim.v2 - prim.v1;

//    NORMALA RAVNI TROUGLA
    vec3 N = cross(u, w);

//    DA LI SU PARALELNE?
    float NdotD = dot(N, r.dir);
    if (abs(NdotD) < 1e-6) {
        return false;
    }

//    DA LI JE PRESEK IZA ZRAKA?
    hit_t = dot(prim.v0 - r.origin, N) / NdotD;
    if(hit_t < 0.0001){
        return false;
    }

//    PRESECNA TACKA
    M = r.origin + hit_t * r.dir;

//    DA LI JE U TROUGLU?
    vec3 p0 = M - prim.v0;
    vec3 p1 = M - prim.v1;
    vec3 p2 = M - prim.v2;

//    DA LI JE M SA ISTIH STRANA SVIH IVICA
    bool b1 = dot(cross(u, p0), N) >= 0.0;
    bool b2 = dot(cross(v, p1), N) >= 0.0;
    bool b3 = dot(cross(-w, p2), N) >= 0.0;

//    DA LI SU ZNAKOVI ISTI
    return (b1 == b2) && (b2 == b3);
}

bool box_intersection(const in Ray r, const in vec3 bmin, const in vec3 bmax){
    float tmin = 0.0;
    float tmax = 1e30;

    for(int d = 0; d < 3; d++){
        bool sign = r.dir_inv[d] < 0.0;
        float b0 = sign ? bmax[d] : bmin[d];
        float b1 = sign ? bmin[d] : bmax[d];
        tmin = max((b0 - r.origin[d]) * r.dir_inv[d], tmin);
        tmax = min((b1 - r.origin[d]) * r.dir_inv[d], tmax);
    }

    return tmin < tmax;
}

bool trace_ray(in vec2 ndc, out vec3 color){
    return false;
}

void main(){
    vec2 ndc = v_uv * 2.0 - 1.0;
    if(trace_ray(ndc)){
//        TODO
    }else{
        FragColor = vec4(0.05, 0.05, 0.08, 1.0);
    }
}
