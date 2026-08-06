#pragma once

#include "engine/resources/Mesh.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <cstdint>
#include <engine/util/BlasTree.hpp>
#include <string>

namespace engine::resources {
class RayTracingModel {
    friend class ResourcesController;

public:
    util::ds::BlasTree m_blas;

    bool is_active() {
        return m_active;
    }

    void activate() {
        m_active = true;
    }

    void deactive() {
        m_active = false;
    }

    void build_bvh() {
        m_blas = util::ds::BlasTree(m_vertices, m_indices, m_textures_indexes);
    }

    uint32_t m_blas_root_offset = 0;

    void translate_model(const glm::vec3 &pos) {
        m_position = pos;
    }

    void rotate_model(const glm::vec3 &rot) {
        m_rotation = rot;
    }

    void scale_model(const glm::vec3 &scale) {
        m_scale = scale;
    }

    glm::mat4 get_local_to_world() const {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position);

        transform = glm::rotate(transform, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        transform = glm::scale(transform, m_scale);

        return transform;
    }

    glm::mat4 get_world_to_local() const {
        return glm::inverse(get_local_to_world());
    }

    const glm::vec3 get_position() const {
        return m_position;
    }

    const glm::vec3 get_rotation() const {
        return m_rotation;
    }

    const glm::vec3 get_scale() const {
        return m_scale;
    }

private:
    bool m_active = false;

    std::string m_name;
    std::string m_path;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<glm::vec4> m_textures_indexes;

    glm::vec3 m_position{0.0f};
    glm::vec3 m_rotation{0.0f};
    glm::vec3 m_scale{1.0f};

    RayTracingModel(
            std::string name,
            std::string path,
            std::vector<Vertex> vertices,
            std::vector<uint32_t> indices,
            std::vector<glm::vec4> texutres_indexes)
        : m_name(std::move(name))
        , m_path(std::move(path))
        , m_vertices(std::move(vertices))
        , m_indices(std::move(indices))
        , m_textures_indexes(std::move(texutres_indexes)) {};
};

}// namespace engine::resources
