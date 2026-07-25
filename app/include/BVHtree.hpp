#include "engine/resources/Model.hpp"
#include "glm/ext/vector_float3.hpp"

namespace engine::resources {
class BVHTree {
public:
    BVHTree(const Model &model);
    ~BVHTree() = default;

private:
    struct Primitive {

        glm::vec3 centroid{};
        glm::vec3 max_bound{};
        glm::vec3 min_bound{};
    };
};
}// namespace engine::resources
