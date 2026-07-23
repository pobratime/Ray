#include "MainController.hpp"
#include "engine/platform/Input.hpp"
#include "engine/platform/PlatformController.hpp"

namespace engine::main::app {
void MainController::initialize() {
    // TODO
}

bool MainController::loop() {
    return m_running;
}

void MainController::begin_draw() {
    // TODO
}

void MainController::draw() {
    // TODO
}

void MainController::end_draw() {
    // TODO
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
