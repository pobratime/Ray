#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/BVHTree.hpp"
#include "engine/resources/Shader.hpp"
#include "engine/util/Errors.hpp"
#include "glad/glad.h"
#include "spdlog/spdlog.h"

namespace engine::graphics {
void RayTracingPipeline::initialize(resources::Shader &shader) {
    // CHECKED_GL_CALL(glGetIntegerv, GL_MAX_TEXTURE_BUFFER_SIZE, &m_max_texture_buffer_texels);
    // spdlog::info("GL_MAX_TEXTURE_BUFFER_SIZE = {}", m_max_texture_buffer_texels);
    CHECKED_GL_CALL(glGenBuffers, 1, &m_node_buffer);
    CHECKED_GL_CALL(glGenTextures, 1, &m_node_texture);

    CHECKED_GL_CALL(glGenBuffers, 1, &m_primitive_buffer);
    CHECKED_GL_CALL(glGenTextures, 1, &m_primitive_texture);

    setup_screen_quad();
}

void RayTracingPipeline::upload(resources::BVHTree &tree) {
    std::vector<float> nodes = tree.serialize_nodes();
    std::vector<float> primitives = tree.serialize_primitives();

    size_t node_texels = nodes.size() / 4;
    size_t primitive_texels = primitives.size() / 4;

    RG_GUARANTEE(node_texels <= static_cast<size_t>(m_max_texture_buffer_texels),
                 "BVH node buffer needs {} texels but GL_MAX_TEXTURE_BUFFER_SIZE is {}.",
                 node_texels, m_max_texture_buffer_texels);
    RG_GUARANTEE(primitive_texels <= static_cast<size_t>(m_max_texture_buffer_texels),
                 "BVH primitive buffer needs {} texels but GL_MAX_TEXTURE_BUFFER_SIZE is {}.",
                 primitive_texels, m_max_texture_buffer_texels);

    CHECKED_GL_CALL(glBindBuffer, GL_TEXTURE_BUFFER, m_node_buffer);
    CHECKED_GL_CALL(glBufferData, GL_TEXTURE_BUFFER, nodes.size() * sizeof(float), nodes.data(), GL_STATIC_DRAW);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, m_node_texture);
    CHECKED_GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGBA32F, m_node_buffer);

    CHECKED_GL_CALL(glBindBuffer, GL_TEXTURE_BUFFER, m_primitive_buffer);
    CHECKED_GL_CALL(glBufferData, GL_TEXTURE_BUFFER, primitives.size() * sizeof(float), primitives.data(), GL_STATIC_DRAW);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, m_primitive_texture);
    CHECKED_GL_CALL(glTexBuffer, GL_TEXTURE_BUFFER, GL_RGBA32F, m_primitive_buffer);
}

void RayTracingPipeline::render() {
    // TODO
    // camera params imgui stuff etc etc
}

void RayTracingPipeline::setup_screen_quad() {
    // TODO
}

}// namespace engine::graphics
