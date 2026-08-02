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
        uint32_t left_child;
        glm::vec3 max_bound;
        uint32_t right_child;
    };

    std::vector<TlasNode> build_tlas(std::vector<resources::RayTracingModel> &rt_models);

private:
    std::vector<TlasNode> m_tlas{};
    unsigned int m_tlas_ssbo;
};
}// namespace engine::util::ds
