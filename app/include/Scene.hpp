#pragma once

#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/Model.hpp"
#include "engine/resources/Shader.hpp"
#include "glm/ext/vector_float3.hpp"

namespace engine::main::app {
class Scene {
public:
    Scene() = default;
    ~Scene() = default;
    void initialize();
    void render();

private:
    void setup_screen_quad();
    // resources::Shader *m_scene_shader = nullptr;
    resources::Model *m_utah_teapod_model = nullptr;
    resources::Model *m_stanford_bunny_model = nullptr;

    graphics::RayTracingPipeline m_pipeline;
    // resources::BVHTree m_teapod_bvh;
    // resources::BVHTree m_bunny_bvh;
};
}// namespace engine::main::app
