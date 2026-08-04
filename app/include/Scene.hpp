#pragma once


#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/RayTracingModel.hpp"
#include "engine/resources/Shader.hpp"

namespace engine::main::app {
class Scene {
public:
    Scene() = default;
    ~Scene() = default;
    void initialize();
    void render();

private:
    void setup_screen_quad();
    float m_dt_acc = 0.0f;
    resources::RayTracingModel *m_bunny{};
    resources::RayTracingModel *m_teapot{};
    resources::Shader *m_shader{};
    graphics::RayTracingPipeline m_pipeline{};
};
}// namespace engine::main::app
