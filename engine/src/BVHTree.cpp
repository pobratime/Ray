#include "engine/util/BVHTree.hpp"
#include "engine/resources/Mesh.hpp"
#include "engine/resources/Texture.hpp"
#include "glm/common.hpp"
#include "glm/ext/vector_float3.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

// TODO istraziti SAH algoritam

namespace engine::util::ds {
BVHTree::BVHTree(const std::vector<engine::resources::Vertex> &vertices,
                 const std::vector<uint32_t> &indices) {
    // TODO ALL OVER :(
}

void BVHTree::build() {
    if (m_primitives.empty()) {
        // TODO ovde mozda moze nesto malo lepse da se cekira i srediti assert - istraziti
        return;
    }
    m_nodes.reserve(std::ceil(2 * std::pow(2, (log2(m_primitives.size()))) - 1));
    build_recursive(0, static_cast<uint32_t>(m_primitives.size()));
}

uint32_t BVHTree::build_recursive(uint32_t start, uint32_t end) {
    Node node{};
    compute_bounds(start, end, node.min_bound, node.max_bound);

    uint32_t node_index = static_cast<uint32_t>(m_nodes.size());
    uint32_t prim_count = end - start;
    m_nodes.push_back(node);

    if (prim_count <= MAX_LEAF_PRIMITIVES) {
        m_nodes[node_index].first_primitive = start;
        m_nodes[node_index].primitive_count = prim_count;
        return node_index;
    }

    glm::vec3 extent = node.max_bound - node.min_bound;
    // along which axis are we splitting ????
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;

    uint32_t mid = start + prim_count / 2;
    std::nth_element(
            m_primitives.begin() + start,
            m_primitives.begin() + mid,
            m_primitives.begin() + end,
            [axis](const Primitive &a, const Primitive &b) {
                return a.centroid[axis] < b.centroid[axis];
            });


    uint32_t left_index = build_recursive(start, mid);
    uint32_t right_index = build_recursive(mid, end);
    m_nodes[node_index].left_child = left_index;
    m_nodes[node_index].right_child = right_index;
    m_nodes[node_index].primitive_count = 0;

    return node_index;
}

void BVHTree::compute_bounds(uint32_t start, uint32_t end,
                             glm::vec3 &min_bound, glm::vec3 &max_bound) {
    min_bound = glm::vec3(std::numeric_limits<float>::max());
    max_bound = glm::vec3(std::numeric_limits<float>::lowest());
    for (uint32_t i = start; i < end; i++) {
        min_bound = glm::min(min_bound, m_primitives[i].min_bound);
        max_bound = glm::max(max_bound, m_primitives[i].max_bound);
    }
}


// OLD
// OLD
// OLD
// OLD

// data.push_back(0.0f); -> PADDINGS FOR SAFETY

// std::vector<float> BVHTree::serialize_nodes() const {
//     std::vector<float> data;
//     data.reserve(m_nodes.size() * 12);
//     for (const Node &node: m_nodes) {
//         data.push_back(node.min_bound.x);
//         data.push_back(node.min_bound.y);
//         data.push_back(node.min_bound.z);
//         data.push_back(0.0f);
//         //
//         data.push_back(node.max_bound.x);
//         data.push_back(node.max_bound.y);
//         data.push_back(node.max_bound.z);
//         data.push_back(0.0f);
//         //
//         data.push_back(glm::uintBitsToFloat(node.left_child));
//         data.push_back(glm::uintBitsToFloat(node.right_child));
//         data.push_back(glm::uintBitsToFloat(node.first_primitive));
//         data.push_back(glm::uintBitsToFloat(node.primitive_count));
//     }
//     return data;
// }

// std::vector<float> BVHTree::serialize_primitives() const {
//     std::vector<float> data;
//     data.reserve(m_primitives.size() * 24);
//     for (const Primitive &prim: m_primitives) {
//         data.push_back(prim.v0.x);
//         data.push_back(prim.v0.y);
//         data.push_back(prim.v0.z);
//         data.push_back(0.0f);
//         //
//         data.push_back(prim.n0.x);
//         data.push_back(prim.n0.y);
//         data.push_back(prim.n0.z);
//         data.push_back(0.0f);
//         //
//         data.push_back(prim.v1.x);
//         data.push_back(prim.v1.y);
//         data.push_back(prim.v1.z);
//         data.push_back(0.0f);
//         //
//         data.push_back(prim.n1.x);
//         data.push_back(prim.n1.y);
//         data.push_back(prim.n1.z);
//         data.push_back(0.0f);
//         //
//         data.push_back(prim.v2.x);
//         data.push_back(prim.v2.y);
//         data.push_back(prim.v2.z);
//         data.push_back(0.0f);
//         //
//         data.push_back(prim.n2.x);
//         data.push_back(prim.n2.y);
//         data.push_back(prim.n2.z);
//         data.push_back(0.0f);
//     }
//     return data;
// }
}// namespace engine::util::ds
