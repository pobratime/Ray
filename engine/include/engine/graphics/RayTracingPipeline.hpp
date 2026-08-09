#pragma once

#include "engine/util/TlasTree.hpp"
#include <cstddef>
#include <cstdint>

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize();
    void render();
    void destroy();

private:
    uint32_t m_global_primitives_ssbo = 0;
    uint32_t m_global_blas_ssbo = 0;
    uint32_t m_tlas_ssbo = 0;
    uint32_t m_instances_ssbo = 0;
    uint32_t m_lights_ssbo = 0;
    uint32_t m_texture_array_id = 0;

    void upload_global_data();
    void update_tlas();
    void update_lights();
    void setup_screen_quad();
    void bind_textures();
    void upload_and_bind_tlas(const util::ds::TlasTree &tlas_tree);
    void upload_and_bind_lights(const std::vector<glm::vec4> &light_srcs);

    static constexpr uint32_t GLOBAL_BLAS_BINDING = 0;
    static constexpr uint32_t GLOBAL_PRIMITIVES_BINDING = 1;
    static constexpr uint32_t TLAS_BINDING = 2;
    static constexpr uint32_t INSTANCE_BINDING = 3;
    static constexpr uint32_t LIGHTS_BINDING = 4;

    size_t m_last_tlas_size = 0;
    size_t m_last_instances_size = 0;
    size_t m_last_lights_size = 0;

    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
};
}// namespace engine::graphics
