#pragma once


#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/Model.hpp"
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
    std::vector<resources::Model *> models{};
    resources::Model *m_bunny{};
    resources::Model *m_teapot{};
    resources::Shader *m_shader{};
    graphics::RayTracingPipeline m_pipeline{};
};
}// namespace engine::main::app
