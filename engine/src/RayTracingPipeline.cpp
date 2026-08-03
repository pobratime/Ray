// clang-format off
#include <glad/glad.h>
// clang-format on
#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/RayTracingModel.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "engine/util/BlasTree.hpp"
#include "engine/util/ThreadPool.hpp"
#include <future>
#include <ranges>// use maybe?
#include <vector>

namespace engine::graphics {
void RayTracingPipeline::initialize() {
    upload_global_blas();
    upload_global_primitives();
    setup_screen_quad();
}

void RayTracingPipeline::render() {
    upload_and_bind_tlas();
    bind_global_blas();
    bind_global_primitives();
    // OPENGL CALLS
}

void RayTracingPipeline::upload_and_bind_tlas() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    std::vector<resources::RayTracingModel *> rtmodels = res_con->rtmodels();
    std::vector<resources::RayTracingModel *> active_rtmodels{};
    for (auto &r: rtmodels) {
        if (r->is_active()) {
            active_rtmodels.push_back(r);
        }
    }
    util::ds::TlasTree tlas_tree(active_rtmodels);
}

void RayTracingPipeline::upload_global_blas() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    util::parallel::ThreadPool pool;
    std::vector<resources::RayTracingModel *> rtmodels = res_con->rtmodels();
    std::vector<std::future<void>> futures{};
    for (auto &r: rtmodels) {
        pool.enqueue([r] {
            r->build_bvh();
        });
    }
    for (auto &f: futures) {
        f.get();
    }

    // maybe do a std::move here?

    // upload part
    std::vector<util::ds::BlasTree::BlasNode> nodes{};
    for (auto &r: rtmodels) {
        const auto blas = r->m_blas.nodes();
        nodes.insert(nodes.end(), blas.begin(), blas.end());
    }

    // don't forget blass_root_offset !!!
    // OPENGL CALLS
}

void RayTracingPipeline::upload_global_primitives() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    std::vector<resources::RayTracingModel *> rtmodels = res_con->rtmodels();
    std::vector<util::ds::BlasTree::GPUPrimitive> primitives{};
    for (auto &r: rtmodels) {
        // primitives.insert(primitives.end(), r->m_blas.nodes().begin(), r->m_blas.nodes().end());
    }
    // maybe do a std::move here?
    // OPENGL CALLS
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
