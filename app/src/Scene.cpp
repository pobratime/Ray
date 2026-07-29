#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/Camera.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/ResourcesController.hpp"
#include "glm/trigonometric.hpp"
#include <cmath>
#include <engine/core/Engine.hpp>

namespace engine::main::app {
void Scene::initialize() {
    const auto resources_controller = engine::core::Controller::get<resources::ResourcesController>();
    // m_scene_shader = resources_controller->shader("scene");
    m_utah_teapod_model = resources_controller->model("utah_teapot");
    m_stanford_bunny_model = resources_controller->model("stanford_bunny");
    m_pipeline.initialize();
}

void Scene::render() {
    // m_scene_shader->use();
    // m_pipeline.bind_resources();
    // m_scene_shader->set_int("u_nodes", 0);
    // m_scene_shader->set_int("u_primitives", 1);
    // const auto graphics_controller = engine::core::Controller::get<graphics::GraphicsController>();
    // const auto camera = graphics_controller->camera();
    // m_scene_shader->set_vec3("u_camera_pos", camera->Position);
    // m_scene_shader->set_vec3("u_camera_front", camera->Front);
    // m_scene_shader->set_vec3("u_camera_up", camera->Up);
    // m_scene_shader->set_vec3("u_camera_right", camera->Right);
    // const float fov_tan = tanf(glm::radians(camera->Zoom / 2.0f));
    // m_scene_shader->set_float("u_fov_tan", fov_tan);
    // m_scene_shader->set_float("u_aspect_ratio", 800.0f / 600.0f);
    // // TODO ...
    // m_pipeline.draw();
}

}// namespace engine::main::app
