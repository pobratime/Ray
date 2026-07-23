#include "CameraController.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include <engine/core/Engine.hpp>

namespace engine::main {
void CameraController::initialize() {
    const auto graphics_controller = get<graphics::GraphicsController>();
    graphics_controller->camera()->Position = {0.0f, 0.0f, 0.0f};
}

void CameraController::poll_events() {
    // TODO
}

void CameraController::update() {
    // TODO
}
}// namespace engine::main
