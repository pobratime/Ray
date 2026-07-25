#include "BVHtree.hpp"
#include "engine/resources/Mesh.hpp"
#include "engine/resources/Model.hpp"

namespace engine::resources {
BVHTree::BVHTree(const Model &model) {
    const std::vector<Mesh> &meshes = model.meshes();
    for (const Mesh &mesh: meshes) {
        // TODO implement tree building
        const auto x = mesh;
    }
}

}// namespace engine::resources
