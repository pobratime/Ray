#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/core/Engine.hpp"
#include "engine/resources/Model.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <vector>

namespace engine::main::app {
void Scene::initialize() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    m_bunny = res_con->model("stanford_bunny");
    m_teapot = res_con->model("utah_teapot");
    m_shader = res_con->shader("ray");
    models.push_back(m_bunny);
    models.push_back(m_teapot);
}

void Scene::render() {
    m_shader->use();
    m_pipeline.bind_resources(models);
}

}// namespace engine::main::app
