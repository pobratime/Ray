#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <vector>

namespace engine::main::app {
void Scene::initialize() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    m_pipeline.initialize();
    m_bunny = res_con->model("stanford_bunny");
    m_teapot = res_con->model("utah_teapot");
    m_shader = res_con->shader("ray");
    models.push_back(m_bunny);
    models.push_back(m_teapot);
}

void Scene::render() {
    m_shader->use();
    m_pipeline.bind_resources(models);
    m_shader->set_vec3("u_camera_position", glm::vec3(0.0f, 1.0f, 4.0f));
    m_shader->set_vec3("u_camera_front", glm::normalize(glm::vec3(0.0f, -0.2f, -1.0f)));
    m_shader->set_vec3("u_camera_up", glm::vec3(0.0f, 1.0f, 0.0f));
    m_shader->set_vec3("u_camera_right", glm::vec3(1.0f, 0.0f, 0.0f));
    m_shader->set_float("u_fov_tan", tanf(glm::radians(45.0f)));
    m_shader->set_float("u_aspect_ratio", 800.0f / 600.0f);
}

}// namespace engine::main::app
