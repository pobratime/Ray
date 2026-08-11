#pragma once

#include "engine/core/Controller.hpp"

namespace engine::main::app {
class MainController final : public core::Controller {
public:
private:
    void initialize() override;
    bool loop() override;
    void poll_events() override;
    void update() override;
    void begin_draw() override;
    void draw() override;
    void end_draw() override;

    bool m_running = true;
};
}// namespace engine::main::app
