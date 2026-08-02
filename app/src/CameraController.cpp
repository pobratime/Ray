#include "CameraController.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include <engine/core/Engine.hpp>

namespace engine::main {
void CameraController::initialize() {
    const auto graphics_controller = get<graphics::GraphicsController>();
    graphics_controller->camera()->Position = {-15.0f, 0.0f, 0.0f};
    const auto platform_controller = get<platform::PlatformController>();
    platform_controller->set_enable_cursor(m_cursor_locked);
}

void CameraController::poll_events() {
    using namespace engine::platform;
    const auto platform_controller = get<platform::PlatformController>();
    const float dt = platform_controller->dt();
    auto camera = get<graphics::GraphicsController>()->camera();

    if (platform_controller->key(KEY_F1).state() == Key::State::JustPressed) {
        m_cursor_locked = !m_cursor_locked;
        platform_controller->set_enable_cursor(m_cursor_locked);
    }

    if (platform_controller->key(KEY_W).is_down())
        camera->move_camera(graphics::Camera::Movement::FORWARD, dt);
    if (platform_controller->key(KEY_S).is_down())
        camera->move_camera(graphics::Camera::Movement::BACKWARD, dt);
    if (platform_controller->key(KEY_A).is_down())
        camera->move_camera(graphics::Camera::Movement::LEFT, dt);
    if (platform_controller->key(KEY_D).is_down())
        camera->move_camera(graphics::Camera::Movement::RIGHT, dt);

    if (!m_cursor_locked) {
        camera->rotate_camera(platform_controller->mouse().dx,
                              platform_controller->mouse().dy);
        camera->zoom(platform_controller->mouse().scroll);
    }
}

}// namespace engine::main
