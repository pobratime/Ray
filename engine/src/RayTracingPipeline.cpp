// clang-format off
#include <glad/glad.h>
// clang-format on
#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/RayTracingModel.hpp"
#include <vector>

namespace engine::graphics {
void RayTracingPipeline::initialize() {
    setup_screen_quad();
}

void RayTracingPipeline::register_models(std::vector<resources::RayTracingModel> &rtmodels) {
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
