#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/BVHTree.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <engine/core/Engine.hpp>

namespace engine::main::app {
void Scene::initialize() {
    const auto resources_controller = engine::core::Controller::get<resources::ResourcesController>();
    m_scene_shader = resources_controller->shader("scene");
    m_utah_teapod_model = resources_controller->model("utah_teapot");
    // m_stanford_bunny_model = resources_controller->model("stanford_bunny");
    m_teapod_bvh = std::make_unique<resources::BVHTree>(*m_utah_teapod_model);
    // m_bunny_bvh = std::make_unique<resources::BVHTree>(*m_stanford_bunny_model);
    pipeline.initialize();
    pipeline.upload(*m_teapod_bvh);
    // pipeline.upload(*m_bunny_bvh);
}

void Scene::render() {
}

}// namespace engine::main::app
