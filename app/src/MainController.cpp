#include "MainController.hpp"
#include "Scene.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/platform/Input.hpp"
#include "engine/platform/PlatformController.hpp"

namespace engine::main::app {
void MainController::initialize() {
    m_scene.initialize();
}

bool MainController::loop() {
    return m_running;
}

void MainController::begin_draw() {
    graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    m_scene.render();
}

void MainController::end_draw() {
    get<platform::PlatformController>()->swap_buffers();
}

void MainController::update() {
    // TODO
}

void MainController::poll_events() {
    using namespace platform;
    auto platform_controller = get<PlatformController>();
    if (platform_controller->key(KEY_BACKSLASH).state() == Key::State::JustPressed) {
        m_running = false;
    }
}
}// namespace engine::main::app
