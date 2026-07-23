#include "MainController.hpp"
#include "engine/core/App.hpp"
#include "engine/core/Controller.hpp"
#include <Main.hpp>

namespace engine::main::app {
// TODO OVDE STA
void MainApp::app_setup() {
    const auto main_controller = register_controller<MainController>();
    main_controller->after(core::Controller::get<core::EngineControllersEnd>());
    main_controller->set_enable(true);
}

int main(const int argc, char **argv) {
    return std::make_unique<MainApp>()->run(argc, argv);
}
}// namespace engine::main::app
