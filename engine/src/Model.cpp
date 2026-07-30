
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::resources {

void Model::draw(const Shader *shader) {
    shader->use();
    for (auto &mesh: m_meshes) {
        mesh.draw(shader);
    }
}

void Model::bind(const unsigned int primitive_slot, const unsigned int node_slot) {
    m_bvh->bind(primitive_slot, node_slot);
}

void Model::destroy() {
    for (auto &mesh: m_meshes) {
        mesh.destroy();
    }
    m_bvh.reset();
}
}// namespace engine::resources
