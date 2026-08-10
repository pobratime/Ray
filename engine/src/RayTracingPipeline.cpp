// clang-format off
#include <glad/glad.h>
// clang-format on
#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/RayTracingModel.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "engine/util/BlasTree.hpp"
#include "engine/util/Errors.hpp"
#include "engine/util/ThreadPool.hpp"
#include "engine/util/TlasTree.hpp"
#include "glm/ext/vector_float3.hpp"
#include <cstddef>
#include <cstdint>
#include <future>
#include <vector>

namespace engine::graphics {
void RayTracingPipeline::initialize(RenderSettings &s) {
    m_settings = s;
    set_settings();
    upload_global_data();
    setup_screen_quad();
    CHECKED_GL_CALL(glEnable, GL_BLEND);
    CHECKED_GL_CALL(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RayTracingPipeline::render(RenderSettings &s) {
    m_settings = s;
    set_settings();
    update_tlas();
    update_lights();
    bind_textures();
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void RayTracingPipeline::set_settings() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    const auto shader = res_con->shader("ray");
    shader->use();
    shader->set_int("u_light_samples", m_settings.light_samples);
    shader->set_int("u_reflection_count", m_settings.reflection_count);
    shader->set_float("u_min_reflection", m_settings.min_reflection);
    shader->set_float("u_light_power", m_settings.light_power);
    shader->set_float("u_ambient", m_settings.ambient);
    shader->set_bool("u_use_textures", m_settings.use_textures);
}

void RayTracingPipeline::bind_textures() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    const auto shader = res_con->shader("ray");
    shader->use();
    const std::vector<resources::Texture *> &textures = res_con->rttextures();
    size_t i = 0;
    for (; i < textures.size(); i++) {
        const int32_t sampler_slot = GL_TEXTURE0 + static_cast<int32_t>(i);
        textures[i]->bind(sampler_slot);
        std::string uniform_name = std::format("u_Textures[{}]", i);
        shader->set_int(uniform_name, static_cast<int32_t>(i));
    }
    // for (; i < textures.size() && i < 32; i++) {
    //     const int32_t sampler_slot = GL_TEXTURE0 + static_cast<int32_t>(i);
    //     textures[i]->bind(sampler_slot);
    //     std::string uniform_name = std::format("u_Textures2[{}]", i);
    //     shader->set_int(uniform_name, static_cast<int32_t>(i));
    // }
}

void RayTracingPipeline::update_lights() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    const std::vector<resources::RayTracingModel *> rtmodels = res_con->rtmodels();
    std::vector<glm::vec4> world_centroids{};
    world_centroids.reserve(rtmodels.size());// overshoot
    uint32_t instance_index = 0;
    for (auto &r: rtmodels) {
        if (!r->is_active()) continue;
        const uint32_t my_instance = instance_index++;// must match update_tlas's filter order
        for (const glm::vec3 &l_c: r->centroids()) {
            glm::vec3 w = glm::vec3(r->get_local_to_world() * glm::vec4(l_c + glm::vec3(0.0f, 0.0f, 0.0f), 1.0f));
            world_centroids.emplace_back(w, static_cast<float>(my_instance));
        }
    }
    const auto ray = res_con->shader("ray");
    ray->use();
    ray->set_int("u_light_count", static_cast<int32_t>(world_centroids.size()));
    if (world_centroids.empty()) {
        return;
    }
    if (m_last_lights_size < world_centroids.size()) {
        upload_and_bind_lights(world_centroids);
        return;
    }
    CHECKED_GL_CALL(glNamedBufferSubData,
                    m_lights_ssbo,
                    0,
                    sizeof(glm::vec4) * world_centroids.size(),
                    world_centroids.data());
}

void RayTracingPipeline::upload_and_bind_lights(const std::vector<glm::vec4> &light_srcs) {
    m_last_lights_size = light_srcs.size();
    if (m_lights_ssbo != 0) {
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_lights_ssbo);
    }
    CHECKED_GL_CALL(glCreateBuffers, 1, &m_lights_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_lights_ssbo,
                    sizeof(glm::vec4) * light_srcs.size(),
                    static_cast<const void *>(light_srcs.data()),
                    GL_DYNAMIC_STORAGE_BIT);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, LIGHTS_BINDING, m_lights_ssbo);
}

void RayTracingPipeline::update_tlas() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    const std::vector<resources::RayTracingModel *> rtmodels = res_con->rtmodels();
    std::vector<resources::RayTracingModel *> active_rtmodels{};
    active_rtmodels.reserve(rtmodels.size());// this is an overshoot, not all models may be active, but its better than realloaction
    for (auto &r: rtmodels) {
        if (r->is_active()) {
            active_rtmodels.push_back(r);
        }
    }
    if (active_rtmodels.size() == 0) {
        return;
    }

    const util::ds::TlasTree tlas_tree{active_rtmodels};
    const std::vector<util::ds::TlasTree::TlasNode> &nodes = tlas_tree.nodes();
    const std::vector<util::ds::TlasTree::GPUInstance> &instances = tlas_tree.instances();

    RG_GUARANTEE(!active_rtmodels.empty(), "No models to ray-trace. Please activate by using [model]->activate().");
    RG_GUARANTEE(!nodes.empty(), "Tlas tree empty.");
    RG_GUARANTEE(!instances.empty(), "GPUInstances for Tlas tree empty.");
    RG_GUARANTEE((sizeof(util::ds::TlasTree::TlasNode) % 16 == 0),
                 "Bad SSBO element alignment for TlasTree::TlasNode.");
    RG_GUARANTEE((sizeof(util::ds::TlasTree::GPUInstance) % 16 == 0),
                 "Bad SSBO element alignment for TlasTree::GPUInstance.");

    if (nodes.size() > m_last_tlas_size || instances.size() > m_last_instances_size) {
        upload_and_bind_tlas(tlas_tree);
        return;
    }

    CHECKED_GL_CALL(glNamedBufferSubData,
                    m_tlas_ssbo,
                    0,
                    sizeof(util::ds::TlasTree::TlasNode) * nodes.size(),
                    nodes.data());

    CHECKED_GL_CALL(glNamedBufferSubData,
                    m_instances_ssbo,
                    0,
                    sizeof(util::ds::TlasTree::GPUInstance) * instances.size(),
                    instances.data());
}

void RayTracingPipeline::upload_and_bind_tlas(const util::ds::TlasTree &tlas_tree) {
    const std::vector<util::ds::TlasTree::TlasNode> &nodes = tlas_tree.nodes();
    const std::vector<util::ds::TlasTree::GPUInstance> &instances = tlas_tree.instances();

    m_last_tlas_size = nodes.size();
    m_last_instances_size = instances.size();

    if (m_tlas_ssbo) {
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_tlas_ssbo);
    }
    CHECKED_GL_CALL(glCreateBuffers, 1, &m_tlas_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_tlas_ssbo,
                    sizeof(util::ds::TlasTree::TlasNode) * nodes.size(),
                    static_cast<const void *>(nodes.data()),
                    GL_DYNAMIC_STORAGE_BIT);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, TLAS_BINDING, m_tlas_ssbo);

    if (m_instances_ssbo) {
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_instances_ssbo);
    }
    CHECKED_GL_CALL(glCreateBuffers, 1, &m_instances_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_instances_ssbo,
                    sizeof(util::ds::TlasTree::GPUInstance) * instances.size(),
                    static_cast<const void *>(instances.data()),
                    GL_DYNAMIC_STORAGE_BIT);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, INSTANCE_BINDING, m_instances_ssbo);
}

void RayTracingPipeline::upload_global_data() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    const std::vector<resources::RayTracingModel *> &rtmodels = res_con->rtmodels();

    std::vector<std::future<void>> futures{};
    futures.reserve(rtmodels.size());
    util::parallel::ThreadPool &pool = util::parallel::ThreadPool::instance();
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
        const std::vector<util::ds::BlasTree::BlasNode> &blas_nodes = r->m_blas.nodes();
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

    RG_GUARANTEE(!nodes.empty(), "Global Blas tree empty");
    RG_GUARANTEE(!primitives.empty(), "Global Primitives for Blas tree empty.");
    RG_GUARANTEE((sizeof(util::ds::BlasTree::BlasNode) % 16 == 0),
                 "Bad SSBO element alignment for BlasTree::BlasNode.");
    RG_GUARANTEE((sizeof(util::ds::BlasTree::GPUPrimitive) % 16 == 0),
                 "Bad SSBO element alignment for BlasTree::GPUPrimitive.");

    CHECKED_GL_CALL(glCreateBuffers, 1, &m_global_blas_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_global_blas_ssbo,
                    sizeof(util::ds::BlasTree::BlasNode) * nodes.size(),
                    static_cast<const void *>(nodes.data()),
                    GL_DYNAMIC_STORAGE_BIT);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, GLOBAL_BLAS_BINDING, m_global_blas_ssbo);

    CHECKED_GL_CALL(glCreateBuffers, 1, &m_global_primitives_ssbo);
    CHECKED_GL_CALL(glNamedBufferStorage,
                    m_global_primitives_ssbo,
                    sizeof(util::ds::BlasTree::GPUPrimitive) * primitives.size(),
                    static_cast<const void *>(primitives.data()),
                    GL_DYNAMIC_STORAGE_BIT);
    CHECKED_GL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, GLOBAL_PRIMITIVES_BINDING,
                    m_global_primitives_ssbo);
}

void RayTracingPipeline::setup_screen_quad() {
    static constexpr float quad_vertices[] = {
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
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                    static_cast<const void *>(nullptr));

    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                    reinterpret_cast<const void *>(2 * sizeof(float)));

    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void RayTracingPipeline::destroy() {
    if (m_quad_vao != 0)
        CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_quad_vao);
    m_quad_vao = 0;
    if (m_quad_vbo != 0)
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_quad_vbo);
    m_quad_vbo = 0;
    if (m_global_primitives_ssbo != 0)
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_global_primitives_ssbo);
    m_global_primitives_ssbo = 0;
    if (m_global_blas_ssbo != 0)
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_global_blas_ssbo);
    m_global_blas_ssbo = 0;
    if (m_tlas_ssbo != 0)
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_tlas_ssbo);
    m_tlas_ssbo = 0;
    if (m_instances_ssbo != 0)
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_instances_ssbo);
    m_instances_ssbo = 0;
}
}// namespace engine::graphics
