#pragma once

#include "engine/resources/RayTracingModel.hpp"
#include "engine/util/BlasTree.hpp"
#include "engine/util/TlasTree.hpp"
#include <vector>

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize();
    void register_models(std::vector<resources::RayTracingModel> &rtmodels);
    void render();

private:
    unsigned int m_global_primitives_ssbo = 0;
    unsigned int m_global_blas_ssbo = 0;

    void setup_screen_quad();
    std::vector<util::ds::BlasTree> create_blas_trees();
    void upload_global_blas();
    void upload_global_primitives();
    void upload_and_bind_tlas(util::ds::TlasTree &tlas_tree);

    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
};
}// namespace engine::graphics
