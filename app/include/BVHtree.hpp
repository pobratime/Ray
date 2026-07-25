#pragma once

#include "engine/resources/Model.hpp"
#include "glm/ext/vector_float3.hpp"
#include <cstdint>

namespace engine::resources {
class BVHTree {
public:
    explicit BVHTree(const Model &model);
    ~BVHTree() = default;

private:
    static constexpr uint32_t MAX_LEAF_PRIMITIVES = 4;

    struct Node {
        glm::vec3 min_bound{};
        glm::vec3 max_bound{};
        uint32_t left_child = 0;
        // right child is left + 1
        // from where to where
        uint32_t first_primitive = 0;
        uint32_t primitive_count = 0;
    };
    struct Primitive {
        glm::vec3 v0, v1, v2;
        glm::vec3 n0, n1, n2;
        glm::vec3 centroid{};
        glm::vec3 max_bound{};
        glm::vec3 min_bound{};
    };

    std::vector<Primitive> m_primitives{};
    std::vector<Node> m_nodes{};

    void build();
    uint32_t build_recursive(uint32_t start, uint32_t end);
    void compute_bounds(uint32_t start, uint32_t end,
                        glm::vec3 &min_bound, glm::vec3 &max_bound);
};
}// namespace engine::resources
