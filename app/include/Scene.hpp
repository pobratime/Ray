#pragma once

#include "engine/core/Engine.hpp"
#include "engine/resources/Shader.hpp"

namespace engine::main::app {
class Scene {
public:
    void initialize();
    void render();

private:
    void setup_screen_quad();
    resources::Shader *m_scene_shader = nullptr;
};
}// namespace engine::main::app
