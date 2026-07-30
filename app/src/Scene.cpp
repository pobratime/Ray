#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <vector>

namespace engine::main::app {
void Scene::initialize() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    m_pipeline.initialize();
    // m_bunny = res_con->model("stanford_bunny");
    m_teapot = res_con->model("utah_teapot");
    m_shader = res_con->shader("ray");
    // models.push_back(m_bunny);
    models.push_back(m_teapot);
}

void Scene::render() {
}

}// namespace engine::main::app
