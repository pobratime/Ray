#include "MainController.hpp"
#include "CameraHandler.hpp"
#include "engine/platform/Input.hpp"
#include "engine/platform/PlatformController.hpp"

namespace engine::main::app {
void MainController::initialize() {
}

bool MainController::loop() {
    return m_running;
}

void MainController::begin_draw() {
}

void MainController::draw() {
}

void MainController::end_draw() {
}

void MainController::update() {
}

void MainController::poll_events() {
    using namespace platform;
    auto platform = get<PlatformController>();

    if (platform->key(KEY_BACKSLASH).state() == Key::State::JustPressed) {
        m_running = false;
    }
}
}// namespace engine::main::app
