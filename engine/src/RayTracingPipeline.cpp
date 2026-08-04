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
#include "engine/util/TlasTree.hpp"
#include <cstdint>
#include <future>
#include <vector>

namespace engine::graphics {
void RayTracingPipeline::initialize() {
    upload_global_data();
    setup_screen_quad();
}

void RayTracingPipeline::render() {
    upload_and_bind_tlas();
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
    CHECKED_GL_CALL(glBindVertexArray, 0);
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
    std::vector<util::ds::TlasTree::TlasNode> nodes = tlas_tree.nodes();
    const std::vector<util::ds::TlasTree::GPUInstance> &instances = tlas_tree.instances();

    if (m_tlas_ssbo) {
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_tlas_ssbo);
    }
    CHECKED_GL_CALL(glCreateBuffers, 1, &m_tlas_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_tlas_ssbo,
                    sizeof(util::ds::TlasTree::TlasNode) * nodes.size(),
                    static_cast<const void *>(nodes.data()),
                    GL_DYNAMIC_STORAGE_BIT);

    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, 2, m_tlas_ssbo);

    if (m_instances_ssbo) {
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_instances_ssbo);
    }
    CHECKED_GL_CALL(glCreateBuffers, 1, &m_instances_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_instances_ssbo,
                    sizeof(util::ds::TlasTree::GPUInstance) * instances.size(),
                    static_cast<const void *>(instances.data()),
                    GL_DYNAMIC_STORAGE_BIT);

    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, 3, m_instances_ssbo);
}

void RayTracingPipeline::upload_global_data() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();

    std::vector<resources::RayTracingModel *> rtmodels = res_con->rtmodels();

    std::vector<std::future<void>> futures{};
    futures.reserve(rtmodels.size());
    util::parallel::ThreadPool pool;
    for (auto &r: rtmodels) {
        futures.push_back(pool.enqueue([r] {
            r->build_bvh();
        }));
    }
    for (auto &f: futures) {
        f.get();
    }

    std::vector<util::ds::BlasTree::BlasNode> nodes{};
    uint32_t node_offset = 0;

    std::vector<util::ds::BlasTree::GPUPrimitive> primitives{};
    uint32_t primitive_offset = 0;

    for (auto &r: rtmodels) {
        const auto blas_nodes = r->m_blas.nodes();
        r->m_blas_root_offset = node_offset;
        for (const auto &n: blas_nodes) {
            util::ds::BlasTree::BlasNode node = n;
            if (node.primitive_count > 0) {
                node.first_primitive += primitive_offset;
            } else {
                node.left_child += node_offset;
                node.right_child += node_offset;
            }
            nodes.push_back(node);
        }
        primitives.insert(primitives.end(), r->m_blas.primitives().begin(), r->m_blas.primitives().end());
        node_offset += static_cast<uint32_t>(blas_nodes.size());
        primitive_offset += static_cast<uint32_t>(r->m_blas.primitives().size());
    }

    CHECKED_GL_CALL(glCreateBuffers, 1, &m_global_blas_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_global_blas_ssbo,
                    sizeof(util::ds::BlasTree::BlasNode) * nodes.size(),
                    static_cast<const void *>(nodes.data()),
                    GL_DYNAMIC_STORAGE_BIT);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, 0, m_global_blas_ssbo);

    CHECKED_GL_CALL(glCreateBuffers, 1, &m_global_primitives_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_global_primitives_ssbo,
                    sizeof(util::ds::BlasTree::GPUPrimitive) * primitives.size(),
                    static_cast<const void *>(primitives.data()),
                    GL_DYNAMIC_STORAGE_BIT);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, 1, m_global_primitives_ssbo);
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
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<const void *>(nullptr));

    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    CHECKED_GL_CALL(glBindVertexArray, 0);
}

}// namespace engine::graphics
