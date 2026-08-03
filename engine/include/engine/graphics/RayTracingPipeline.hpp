#pragma once

#include "engine/util/BlasTree.hpp"
#include "engine/util/TlasTree.hpp"

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize();
    void render();

private:
    unsigned int m_global_primitives_ssbo = 0;
    unsigned int m_global_blas_ssbo = 0;

    void setup_screen_quad();
    void upload_global_blas();
    void upload_global_primitives();
    void bind_global_blas();
    void bind_global_primitives();
    void upload_and_bind_tlas();

    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
};
}// namespace engine::graphics
