#pragma once

#include "engine/resources/Mesh.hpp"
#include <cstdint>
#include <vector>

namespace engine::util::ds {
class BlasTree {
    friend class TlasTree;

public:
    BlasTree() = default;

    explicit BlasTree(const std::vector<engine::resources::Vertex> &vertices,
                      const std::vector<uint32_t> &indices,
                      const std::vector<glm::vec4> &texutres_indexes);

    struct Bounds {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct BlasNode {
        glm::vec3 min_bound;
        float pad0;
        glm::vec3 max_bound;
        float pad1;
        uint32_t left_child = 0;
        uint32_t right_child = 0;
        uint32_t first_primitive = 0;
        uint32_t primitive_count = 0;
    };

    struct GPUPrimitive {
        glm::vec4 v0, v1, v2;
        glm::vec4 n0, n1, n2;
        glm::vec4 uv0, uv1, uv2;
        glm::vec4 t0, t1, t2;
        glm::vec4 b0, b1, b2;
        glm::vec4 t_idx;
    };

    const std::vector<GPUPrimitive> &primitives() const {
        return m_primitives;
    }

    const std::vector<BlasNode> &nodes() const {
        return m_nodes;
    }

private:
    static constexpr uint32_t MAX_LEAF_PRIMITIVES = 4;

    struct CPUPrimitive {
        glm::vec3 v0, v1, v2;
        glm::vec3 n0, n1, n2;
        glm::vec2 uv0, uv1, uv2;
        glm::vec3 t0, t1, t2;
        glm::vec3 b0, b1, b2;
        glm::vec4 texutre;
        glm::vec3 centroid;
    };

    std::vector<GPUPrimitive> m_primitives{};
    std::vector<BlasNode> m_nodes{};

    uint32_t build_recursive(std::vector<CPUPrimitive> &primitives,
                             uint32_t start, uint32_t end);
    Bounds compute_bounds(uint32_t start, uint32_t end,
                          const std::vector<CPUPrimitive> &primitives);

    std::vector<CPUPrimitive> transform_to_cpu(const std::vector<resources::Vertex> &vertices,
                                               const std::vector<uint32_t> &indices,
                                               const std::vector<glm::vec4> &texutres_indexes);
    std::vector<GPUPrimitive> transform_to_gpu(const std::vector<CPUPrimitive> &primitives);
};
}// namespace engine::util::ds
