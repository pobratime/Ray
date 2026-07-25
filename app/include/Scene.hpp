#pragma once

#include "engine/resources/BVHTree.hpp"
#include "engine/resources/Model.hpp"
#include "engine/resources/Shader.hpp"
#include "glm/ext/vector_float3.hpp"
#include <memory>

namespace engine::main::app {
class Scene {
public:
    Scene() = default;
    ~Scene() = default;
    void initialize();
    void render();

private:
    void setup_screen_quad();
    resources::Shader *m_scene_shader = nullptr;
    resources::Model *m_utah_teapod_model = nullptr;
    resources::Model *m_stanford_bunny_model = nullptr;

    std::unique_ptr<resources::BVHTree> m_teapod_bvh;
    std::unique_ptr<resources::BVHTree> m_bunny_bvh;

    glm::vec3 m_teapod_pos = {0.0f, 0.0f, 0.0f};
    glm::vec3 m_bunny_pos = {0.0f, 0.0f, 0.0f};
};
}// namespace engine::main::app
