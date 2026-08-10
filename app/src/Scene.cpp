#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "engine/platform/PlatformController.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "glm/trigonometric.hpp"
#include "spdlog/spdlog.h"

namespace engine::main {
void Scene::initialize() {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    m_pipeline.initialize(settings);
    m_shader = res_con->shader("ray");
    m_chess = res_con->rtmodel("chess");
    m_desk_lamp = res_con->rtmodel("desk_lamp");
    m_desk_lamp->translate_model(glm::vec3(0.0f, 0.55f, -0.5f));
    m_desk_lamp->activate();
    const auto table = res_con->rtmodel("table");
    table->activate();
    table->scale_model(glm::vec3(1.5f, 1.0f, 3.0f));
    m_chess->translate_model(glm::vec3(0, 0.55f, 0));
    m_chess->activate();
}

void Scene::render() {
    const auto camera = engine::core::Controller::get<graphics::GraphicsController>()->camera();
    const auto window = engine::core::Controller::get<platform::PlatformController>()->window();
    // spdlog::info("camra pos -> {} {} {}", camera->Position.x, camera->Position.y, camera->Position.z);
    // spdlog::info("camera front -> {} {} {}", camera->Front.x, camera->Front.y, camera->Front.z);
    m_shader->use();
    m_pipeline.render(settings);
    m_shader->set_vec3("u_camera_position", camera->Position);
    m_shader->set_vec3("u_camera_front", camera->Front);
    m_shader->set_vec3("u_camera_up", camera->Up);
    m_shader->set_vec3("u_camera_right", camera->Right);
    m_shader->set_float("u_fov_tan", tanf(glm::radians(camera->Zoom)));
    m_shader->set_float("u_aspect_ratio", static_cast<float>(window->width()) / static_cast<float>(window->height()));
}

}// namespace engine::main
