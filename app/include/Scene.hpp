#pragma once

#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/RayTracingModel.hpp"

namespace engine::main::app {
class Scene {
public:
    Scene() = default;
    ~Scene() = default;
    void initialize();
    void render();

private:
    resources::RayTracingModel *m_bunny{};
    resources::RayTracingModel *m_teapot{};
    resources::RayTracingModel *m_backpack{};
    resources::Shader *m_shader{};
    graphics::RayTracingPipeline m_pipeline{};
};
}// namespace engine::main::app
