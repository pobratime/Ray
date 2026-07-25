#pragma once

#include "engine/resources/Model.hpp"
#include "glm/ext/vector_float3.hpp"
#include <cstdint>

namespace engine::resources {
class BVHTree {
public:
    BVHTree(const Model &model);
    ~BVHTree() = default;

private:
    struct Node {
        glm::vec3 min_bound{};
        glm::vec3 max_bound{};
        // > 0 leaf, == 0 inner node nno triangles
        uint32_t left_child = 0;
        // from where to where
        uint32_t first_primitive = 0;
        uint32_t primitive_count = 0;
    };
    struct Primitive {
        glm::vec3 v0, v1, v2;
        glm::vec3 centroid{};
        glm::vec3 max_bound{};
        glm::vec3 min_bound{};
    };

    std::vector<Primitive> m_primitives{};
};
}// namespace engine::resources
