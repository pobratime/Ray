#include "CameraController.hpp"
#include "MainController.hpp"
#include "engine/core/App.hpp"
#include "engine/core/Controller.hpp"
#include "engine/platform/PlatformController.hpp"
#include <Main.hpp>

namespace engine::main::app {
void MainApp::app_setup() {
    const auto main_controller = register_controller<MainController>();
    const auto camera_controller = register_controller<CameraController>();
    main_controller->after(core::Controller::get<core::EngineControllersEnd>());
    main_controller->set_enable(true);
    camera_controller->set_enable(true);
    camera_controller->after(core::Controller::get<platform::PlatformController>());
}
}// namespace engine::main::app

int main(const int argc, char **argv) {
    return std::make_unique<engine::main::app::MainApp>()->run(argc, argv);
}
