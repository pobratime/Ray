#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "glm/trigonometric.hpp"

namespace engine::main::app {
void Scene::initialize() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    m_pipeline.initialize();
    // m_bunny = res_con->rtmodel("stanford_bunny");
    // m_teapot = res_con->rtmodel("utah_teapot");
    m_shader = res_con->shader("ray");
    // m_backpack = res_con->rtmodel("backpack");
    m_mando = res_con->rtmodel("mando");
    // m_chess = res_con->rtmodel("chess");
    m_mando->activate();
    // m_chess->activate();
}

void Scene::render() {
    const auto camera = engine::core::Controller::get<graphics::GraphicsController>()->camera();
    const auto window = engine::core::Controller::get<platform::PlatformController>()->window();
    m_shader->use();
    m_pipeline.render();
    m_shader->set_vec3("u_camera_position", camera->Position);
    m_shader->set_vec3("u_camera_front", camera->Front);
    m_shader->set_vec3("u_camera_up", camera->Up);
    m_shader->set_vec3("u_camera_right", camera->Right);
    m_shader->set_float("u_fov_tan", tanf(glm::radians(camera->Zoom)));
    m_shader->set_float("u_aspect_ratio", static_cast<float>(window->width()) / static_cast<float>(window->height()));
}

}// namespace engine::main::app
