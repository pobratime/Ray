// clang-format off
#include <glad/glad.h>
// clang-format on
#include "engine/util/BVHTree.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/Mesh.hpp"
#include "glm/common.hpp"
#include "glm/ext/vector_float3.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <glad/glad.h>
#include <limits>
#include <vector>

namespace engine::util::ds {
BVHTree::BVHTree(const std::vector<engine::resources::Vertex> &vertices,
                 const std::vector<uint32_t> &indices) {

    m_primitives.reserve(indices.size() / 3);
    transform_to_cpu(vertices, indices);
    m_nodes.reserve(2 * std::pow(2, std::log2(static_cast<uint32_t>(m_primitives.size()))) - 1);
    build_recursive(0, static_cast<uint32_t>(m_primitives.size()));
}

void BVHTree::transform_to_cpu(const std::vector<resources::Vertex> &vertices,
                               const std::vector<uint32_t> &indices) {

    for (uint32_t i = 0; i < indices.size(); i += 3) {
        const auto &v0 = vertices[indices[i]];
        const auto &v1 = vertices[indices[i + 1]];
        const auto &v2 = vertices[indices[i + 2]];

        CPUPrimitive prim{};
        prim.v0 = v0.Position;
        prim.v1 = v1.Position;
        prim.v2 = v2.Position;
        prim.centroid = (prim.v0 + prim.v1 + prim.v2) / 3.0f;

        m_primitives.push_back(prim);
    }
}

void BVHTree::upload() {
    CHECKED_GL_CALL(glGenBuffers, 1, &m_ssbo);
    CHECKED_GL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, m_ssbo);
}
// TODO void BVHTree::bind

uint32_t BVHTree::build_recursive(uint32_t start, uint32_t end) {

    uint32_t node_index = m_nodes.size();
    Node node{};
    m_nodes.push_back(node);
    compute_bounds(start, end, m_nodes[node_index].min_bound, m_nodes[node_index].max_bound);
    if (end - start <= MAX_LEAF_PRIMITIVES) {
        m_nodes[node_index].first_primitive = start;
        m_nodes[node_index].primitive_count = end - start;
        return node_index;
    }

    glm::vec3 diff = m_nodes[node_index].max_bound - m_nodes[node_index].min_bound;

    int axis = 0;                     // x
    if (diff.y > diff.x) axis = 1;    // y
    if (diff.z > diff[axis]) axis = 2;//z
    // forcing leaf case in case cube is degenrated
    if (diff.x == 0.0f && diff.y == 0.0f && diff.z == 0.0f) {
        m_nodes[node_index].first_primitive = start;
        m_nodes[node_index].primitive_count = end - start;
        return node_index;
    }
    const uint32_t mid = (start + end) / 2;
    std::nth_element(
            m_primitives.begin() + start,
            m_primitives.begin() + mid,
            m_primitives.begin() + end,
            [axis](const CPUPrimitive &a, const CPUPrimitive &b) {
                return a.centroid[axis] < b.centroid[axis];
            });
    uint32_t left = build_recursive(start, mid);
    uint32_t right = build_recursive(mid, end);
    m_nodes[node_index].left_child = left;
    m_nodes[node_index].right_child = right;
    m_nodes[node_index].primitive_count = 0;

    return node_index;
}

void BVHTree::compute_bounds(uint32_t start, uint32_t end, glm::vec3 &min, glm::vec3 &max) {
    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(-std::numeric_limits<float>::max());

    for (uint32_t i = start; i < end; i++) {
        const auto &p = m_primitives[i];
        min = glm::min(min, glm::vec3(p.v0));
        min = glm::min(min, glm::vec3(p.v1));
        min = glm::min(min, glm::vec3(p.v2));

        max = glm::max(max, glm::vec3(p.v0));
        max = glm::max(max, glm::vec3(p.v1));
        max = glm::max(max, glm::vec3(p.v2));
    }
}

}// namespace engine::util::ds
