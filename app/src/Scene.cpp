#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <engine/core/Engine.hpp>

namespace engine::main::app {
void Scene::initialize() {
    const auto resources_controller = engine::core::Controller::get<resources::ResourcesController>();
    m_scene_shader = resources_controller->shader("scene");
    m_utah_teapod_model = resources_controller->model("utah_teapot");
    m_stanford_bunny_model = resources_controller->model("stanford_bunny");
}

void Scene::render() {
}

void Scene::setup_screen_quad() {
    // TODO
}

}// namespace engine::main::app
