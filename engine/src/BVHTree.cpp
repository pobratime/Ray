// clang-format off
#include <cmath>
#include <glad/glad.h>
// clang-format on
#include "engine/util/BVHTree.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/Mesh.hpp"
#include "glm/common.hpp"
#include "glm/ext/vector_float3.hpp"
#include <algorithm>
#include <cstdint>
#include <glad/glad.h>
#include <vector>

namespace engine::util::ds {
BVHTree::BVHTree(const std::vector<engine::resources::Vertex> &vertices,
                 const std::vector<uint32_t> &indices) {

    std::vector<CPUPrimitive> primitives = transform_to_cpu(vertices, indices);
    std::vector<Node> nodes = build(primitives);
    std::vector<GPUPrimitive> g_primitives = transform_to_gpu(primitives);
    // THIS HAS TO BE MOVED SINCE OPENGL FUNCTIONS CAN ONLY BE CALLED FROM MAIN THREAD
    upload_to_gpu(g_primitives, nodes);
}

std::vector<BVHTree::CPUPrimitive> BVHTree::transform_to_cpu(const std::vector<resources::Vertex> &vertices,
                                                             const std::vector<uint32_t> &indices) {

    std::vector<CPUPrimitive> primitives;
    primitives.reserve(indices.size() / 3);
    for (uint32_t i = 0; i < indices.size(); i += 3) {
        const auto &v0 = vertices[indices[i]];
        const auto &v1 = vertices[indices[i + 1]];
        const auto &v2 = vertices[indices[i + 2]];

        CPUPrimitive prim{};
        prim.v0 = v0.Position;
        prim.v1 = v1.Position;
        prim.v2 = v2.Position;

        prim.n0 = v0.Normal;
        prim.n1 = v1.Normal;
        prim.n2 = v2.Normal;

        prim.uv0 = v0.TexCoords;
        prim.uv1 = v1.TexCoords;
        prim.uv2 = v2.TexCoords;

        prim.t0 = v0.Tangent;
        prim.t1 = v1.Tangent;
        prim.t2 = v2.Tangent;

        prim.b0 = v0.Bitangent;
        prim.b1 = v1.Bitangent;
        prim.b2 = v2.Bitangent;

        prim.centroid = (prim.v0 + prim.v1 + prim.v2) / 3.0f;

        primitives.push_back(prim);
    }
    return primitives;
}

std::vector<BVHTree::GPUPrimitive> BVHTree::transform_to_gpu(std::vector<CPUPrimitive> &primitives) {
    std::vector<GPUPrimitive> g_primitives{};
    g_primitives.reserve(primitives.size());
    for (size_t i = 0; i < primitives.size(); i++) {
        CPUPrimitive prim = primitives[i];
        GPUPrimitive gpu{};
        gpu.v0 = glm::vec4(prim.v0, 1.0f);
        gpu.v1 = glm::vec4(prim.v1, 1.0f);
        gpu.v2 = glm::vec4(prim.v2, 1.0f);

        gpu.n0 = glm::vec4(prim.n0, 0.0f);
        gpu.n1 = glm::vec4(prim.n1, 0.0f);
        gpu.n2 = glm::vec4(prim.n2, 0.0f);

        gpu.uv0 = glm::vec4(prim.uv0, 0.0f, 0.0f);
        gpu.uv1 = glm::vec4(prim.uv1, 0.0f, 0.0f);
        gpu.uv2 = glm::vec4(prim.uv2, 0.0f, 0.0f);

        gpu.t0 = glm::vec4(prim.t0, 0.0f);
        gpu.t1 = glm::vec4(prim.t1, 0.0f);
        gpu.t2 = glm::vec4(prim.t2, 0.0f);

        gpu.b0 = glm::vec4(prim.b0, 0.0f);
        gpu.b1 = glm::vec4(prim.b1, 0.0f);
        gpu.b2 = glm::vec4(prim.b2, 0.0f);
        g_primitives.push_back(gpu);
    }
    return g_primitives;
}

void BVHTree::upload_to_gpu(std::vector<GPUPrimitive> &g_primitives, std::vector<Node> &nodes) {
    CHECKED_GL_CALL(glCreateBuffers, 1, &m_primitive_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_primitive_ssbo,
                    g_primitives.size() * sizeof(GPUPrimitive),
                    g_primitives.data(),
                    GL_DYNAMIC_STORAGE_BIT);

    CHECKED_GL_CALL(glCreateBuffers, 1, &m_node_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_node_ssbo,
                    nodes.size() * sizeof(Node),
                    nodes.data(),
                    GL_DYNAMIC_STORAGE_BIT);
}

void BVHTree::bind(const unsigned int primitive_slot, const unsigned int node_slot) {
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, primitive_slot, m_primitive_ssbo);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, node_slot, m_node_ssbo);
}

std::vector<BVHTree::Node> BVHTree::build(std::vector<CPUPrimitive> &primitives) {
    std::vector<Node> nodes{};
    nodes.reserve(2 * std::pow(2, log2(primitives.size())) - 1);
    build_recursive(primitives, nodes, 0, static_cast<uint32_t>(primitives.size()));
    return nodes;
}

uint32_t BVHTree::build_recursive(std::vector<CPUPrimitive> &primitives,
                                  std::vector<Node> &nodes,
                                  uint32_t start, uint32_t end) {

    uint32_t node_index = nodes.size();
    Node node{};
    Bounds bounds = compute_bounds(start, end, primitives);
    node.min_bound = bounds.min;
    node.max_bound = bounds.max;
    nodes.push_back(node);
    if (end - start <= MAX_LEAF_PRIMITIVES) {
        nodes[node_index].first_primitive = start;
        nodes[node_index].primitive_count = end - start;
        return node_index;
    }

    glm::vec3 diff = nodes[node_index].max_bound - nodes[node_index].min_bound;

    int axis = 0;                     // x
    if (diff.y > diff.x) axis = 1;    // y
    if (diff.z > diff[axis]) axis = 2;//z
    // forcing leaf case in case cube is degenrated
    if (diff.x == 0.0f && diff.y == 0.0f && diff.z == 0.0f) {
        nodes[node_index].first_primitive = start;
        nodes[node_index].primitive_count = end - start;
        return node_index;
    }
    const uint32_t mid = (start + end) / 2;
    std::nth_element(
            primitives.begin() + start,
            primitives.begin() + mid,
            primitives.begin() + end,
            [axis](const CPUPrimitive &a, const CPUPrimitive &b) {
                return a.centroid[axis] < b.centroid[axis];
            });

    uint32_t left = build_recursive(primitives, nodes, start, mid);
    uint32_t right = build_recursive(primitives, nodes, mid, end);
    nodes[node_index].left_child = left;
    nodes[node_index].right_child = right;
    nodes[node_index].primitive_count = 0;

    return node_index;
}

BVHTree::Bounds BVHTree::compute_bounds(const uint32_t start, const uint32_t end,
                                        const std::vector<CPUPrimitive> &primitives) {
    glm::vec3 min = glm::min(primitives[start].v0, glm::min(primitives[start].v1, primitives[start].v2));
    glm::vec3 max = glm::max(primitives[start].v0, glm::max(primitives[start].v1, primitives[start].v2));
    for (uint32_t i = start + 1; i < end; i++) {
        const auto &p = primitives[i];
        min = glm::min(min, glm::vec3(p.v0));
        min = glm::min(min, glm::vec3(p.v1));
        min = glm::min(min, glm::vec3(p.v2));

        max = glm::max(max, glm::vec3(p.v0));
        max = glm::max(max, glm::vec3(p.v1));
        max = glm::max(max, glm::vec3(p.v2));
    }
    return Bounds{min, max};
}

}// namespace engine::util::ds
