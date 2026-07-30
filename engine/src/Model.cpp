
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::resources {

void Model::draw(const Shader *shader) {
    shader->use();
    for (auto &mesh: m_meshes) {
        mesh.draw(shader);
    }
}

void Model::bind(const unsigned int a, const unsigned int b) {
    m_bvh->bind(a, b);
}

void Model::destroy() {
    for (auto &mesh: m_meshes) {
        mesh.destroy();
    }
    m_bvh.reset();
}
}// namespace engine::resources
