#pragma once

#include "engine/resources/RayTracingModel.hpp"

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize();
    void bind_resources(const std::vector<resources::RayTracingModel *> &models);

private:
    void setup_screen_quad();
    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
};
}// namespace engine::graphics
