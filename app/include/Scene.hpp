#pragma once

#include "engine/core/Controller.hpp"
#include "engine/graphics/Bloom.hpp"
#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/RayTracingModel.hpp"

namespace engine::main {
class Scene final : public core::Controller {
public:
    Scene() = default;
    ~Scene() = default;
    graphics::RayTracingPipeline::RenderSettings settings{};
    graphics::Bloom::BloomSettings bloom_settings{};
    void initialize();
    void render();

private:
    resources::RayTracingModel *m_backpack{};
    resources::RayTracingModel *m_desk_lamp{};
    resources::RayTracingModel *m_chess{};


    resources::Shader *m_shader{};
    graphics::RayTracingPipeline m_pipeline{};
    graphics::Bloom m_bloom{};
};// namespace engine::main
}// namespace engine::main
