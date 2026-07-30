#pragma once

#include "engine/resources/Mesh.hpp"
#include "glm/ext/vector_float3.hpp"
#include <cstdint>
#include <vector>

namespace engine::util::ds {


class BVHTree {
public:
    explicit BVHTree(const std::vector<engine::resources::Vertex> &vertices,
                     const std::vector<uint32_t> &indices);
    ~BVHTree() = default;
    BVHTree() = default;

    void bind(const unsigned int primitive_slot, const unsigned int node_slot);

private:
    static constexpr uint32_t MAX_LEAF_PRIMITIVES = 4;

    struct Bounds {
        glm::vec3 min;
        glm::vec3 max;
    };

    // ALLIGNED FOR SSBO
    struct Node {
        glm::vec3 min_bound;
        float pad0;
        glm::vec3 max_bound;
        float pad1;
        // if == 0 then its a leaf, else node
        uint32_t left_child = 0;
        uint32_t right_child = 0;
        uint32_t first_primitive = 0;
        // if == 0 then its a node, else leaf
        uint32_t primitive_count = 0;
    };

    struct CPUPrimitive {
        glm::vec3 v0, v1, v2;
        glm::vec3 n0, n1, n2;
        glm::vec2 uv0, uv1, uv2;
        glm::vec3 t0, t1, t2;
        glm::vec3 b0, b1, b2;
        glm::vec3 centroid;
    };

    // ALLIGNED FOR SSBO
    struct GPUPrimitive {
        glm::vec4 v0, v1, v2;
        glm::vec4 n0, n1, n2;
        glm::vec4 uv0, uv1, uv2;
        glm::vec4 t0, t1, t2;
        glm::vec4 b0, b1, b2;
    };

    unsigned int m_primitive_ssbo = 0;
    unsigned int m_node_ssbo = 0;

    std::vector<Node> build(std::vector<CPUPrimitive> &primitives);
    uint32_t build_recursive(std::vector<CPUPrimitive> &primitives,
                             std::vector<Node> &nodes,
                             uint32_t start, uint32_t end);
    Bounds compute_bounds(const uint32_t start, const uint32_t end,
                          const std::vector<CPUPrimitive> &primitives);

    std::vector<CPUPrimitive> transform_to_cpu(const std::vector<resources::Vertex> &vertices,
                                               const std::vector<uint32_t> &indices);
    std::vector<GPUPrimitive> transform_to_gpu(std::vector<CPUPrimitive> &primitives);

    void upload_to_gpu(std::vector<GPUPrimitive> &g_primitives, std::vector<Node> &nodes);
};
}// namespace engine::util::ds
