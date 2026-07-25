#include "BVHtree.hpp"
#include "engine/resources/Mesh.hpp"
#include "engine/resources/Model.hpp"
#include "glm/common.hpp"
#include "glm/ext/vector_float3.hpp"
#include <cstdint>

// TODO - IMPLEMENT SAH ALGORITHM IF THERE IS TIME LEFT / IF THIS IS SUCCESSFUL

namespace engine::resources {
BVHTree::BVHTree(const Model &model) {
    const std::vector<Mesh> &meshes = model.meshes();
    for (const Mesh &mesh: meshes) {
        const std::vector<Vertex> &vertices = mesh.vertices();
        const std::vector<uint32_t> &indices = mesh.indices();
        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            const glm::vec3 &p0 = vertices[i0].Position;
            const glm::vec3 &p1 = vertices[i1].Position;
            const glm::vec3 &p2 = vertices[i2].Position;

            Primitive prim{};
            prim.v0 = p0;
            prim.v1 = p1;
            prim.v2 = p2;
            prim.min_bound = glm::min(p0, glm::min(p1, p2));
            prim.max_bound = glm::max(p0, glm::max(p1, p2));
            prim.centroid = (p0 + p1 + p2) / 3.0f;
            m_primitives.push_back(prim);
        }
    }
}


}// namespace engine::resources
