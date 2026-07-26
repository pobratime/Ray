//#shader vertex
#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 v_uv;

void main(){
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0f, 1.0f);
}

//#shader fragment
#version 330 core
in vec2 v_uv;
out vec4 FragColor;

uniform samplerBuffer u_nodes;
uniform samplerBuffer u_primitives;

uniform vec3 u_camera_pos;
uniform vec3 u_camera_front;
uniform vec3 u_camera_up;
uniform vec3 u_camera_right;

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

void main(){

}