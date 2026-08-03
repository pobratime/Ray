#include "engine/resources/RayTracingModel.hpp"
#include "engine/util/BlasTree.hpp"
#include <cstdint>
#include <vector>

namespace engine::util::ds {
class TlasTree {
    friend class BlasTree;

public:
    struct TlasNode {
        glm::vec3 min_bound;
        float pad0;
        glm::vec3 max_bound;
        float pad1;
        uint32_t left_child;
        uint32_t right_child;
        uint32_t first_instance;
        uint32_t instance_count;
    };

    struct GPUInstance {
        glm::mat4 world_to_local;
        glm::mat4 local_to_world;
        uint32_t blas_root_index;
        uint32_t material_index;// TODO change later
        uint32_t pad0;
        uint32_t pad1;
    };

    TlasTree() = default;

    TlasTree(std::vector<resources::RayTracingModel *> &rt_models);

    ~TlasTree() = default;

    const std::vector<TlasNode> &nodes() const {
        return m_nodes;
    }

    const std::vector<GPUInstance> &instances() const {
        return m_instances;
    }

private:
    struct InstanceBounds {
        glm::vec3 min;
        glm::vec3 max;
        glm::vec3 centroid;
        uint32_t instance_index;
    };

    uint32_t build_recursive(std::vector<InstanceBounds> &bounds,
                             uint32_t start, uint32_t end);

    std::vector<TlasNode> m_nodes;
    std::vector<GPUInstance> m_instances;

    InstanceBounds compute_bounds(std::vector<InstanceBounds> &bounds,
                                  uint32_t start, uint32_t end);

    static constexpr uint32_t MAX_LEAF_INSTANCES = 1;
};
}// namespace engine::util::ds
