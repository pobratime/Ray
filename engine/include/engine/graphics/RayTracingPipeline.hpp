#pragma once

#include "engine/resources/RayTracingModel.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "engine/util/BlasTree.hpp"
#include "engine/util/TlasTree.hpp"

namespace engine::graphics {
class RayTracingPipeline {
    friend class resources::ResourcesController;


public:
    void initialize();
    void register_models(std::vector<resources::RayTracingModel> &rtmodels);

private:
    util::ds::TlasTree m_tlas{};

    void setup_screen_quad();
    void create_global_blas(std::vector<resources::RayTracingModel> &rtmodels);
    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
};
}// namespace engine::graphics
