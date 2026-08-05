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

    void upload_global_data();
    void update_tlas();
    void setup_screen_quad();
    void upload_and_bind_tlas(const util::ds::TlasTree &tlas_tree);

    static constexpr uint32_t GLOBAL_BLAS_BINDING = 0;
    static constexpr uint32_t GLOBAL_PRIMITIVES_BINDING = 1;
    static constexpr uint32_t TLAS_BINDING = 2;
    static constexpr uint32_t INSTANCE_BINDIING = 3;

    size_t m_last_tlas_size = 0;
    size_t m_last_instances_size = 0;

    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
};
}// namespace engine::graphics
