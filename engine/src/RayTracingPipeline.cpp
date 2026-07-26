#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/BVHTree.hpp"
#include "engine/util/Errors.hpp"
#include "glad/glad.h"

namespace engine::graphics {
void RayTracingPipeline::initialize() {
    CHECKED_GL_CALL(glGetIntegerv, GL_MAX_TEXTURE_BUFFER_SIZE, &m_max_texture_buffer_texels);
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

void RayTracingPipeline::bind_resources() {
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, m_node_texture);

    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE1);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_BUFFER, m_primitive_texture);
}

void RayTracingPipeline::draw() {
}

void RayTracingPipeline::setup_screen_quad() {
    static const float quad_vertices[] = {
            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            1.0f,
            0.0f,

            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
    };

    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_quad_vao);
    CHECKED_GL_CALL(glGenBuffers, 1, &m_quad_vbo);

    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_quad_vbo);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    CHECKED_GL_CALL(glBindVertexArray, 0);
}

}// namespace engine::graphics
