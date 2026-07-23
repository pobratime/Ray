#pragma once

#include <engine/core/Engine.hpp>

namespace engine::main {
class CameraController final : public core::Controller {
public:
    void get_camera();

private:
    void poll_events() override;
    void initialize() override;
    void update() override;
};
}// namespace engine::main
