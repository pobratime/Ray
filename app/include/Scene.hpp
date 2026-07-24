#pragma once

#include "engine/resources/Model.hpp"
#include "engine/resources/Shader.hpp"

namespace engine::main::app {
class Scene {
public:
    Scene() = default;
    ~Scene();
    void initialize();
    void render();

private:
    void setup_screen_quad();
    resources::Shader *m_scene_shader = nullptr;
    resources::Model *m_utah_teapod_model = nullptr;
};
}// namespace engine::main::app
