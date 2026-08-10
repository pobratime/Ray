#include "CameraController.hpp"
#include "MainController.hpp"
#include "Scene.hpp"
#include "engine/core/App.hpp"
#include "engine/core/Controller.hpp"
#include "engine/platform/PlatformController.hpp"
#include <Main.hpp>

#include "ImGuiController.hpp"

namespace engine::main::app {
void MainApp::app_setup() {
    const auto main_controller = register_controller<MainController>();
    const auto camera_controller = register_controller<CameraController>();
    const auto imgui_controller = register_controller<ImGuiController>();
    const auto scene_controller = register_controller<Scene>();
    main_controller->set_enable(true);
    camera_controller->set_enable(true);
    imgui_controller->set_enable(true);
    main_controller->after(core::Controller::get<core::EngineControllersEnd>());
    camera_controller->after(main_controller);
    imgui_controller->after(camera_controller);
    scene_controller->after(imgui_controller);
}
}// namespace engine::main::app

int main(const int argc, char **argv) {
    return std::make_unique<engine::main::app::MainApp>()->run(argc, argv);
}
