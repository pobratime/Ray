#pragma once

#include <engine/core/Engine.hpp>

namespace engine::main {
class CameraController final : public core::Controller {
public:
private:
    void poll_events() override;
    void initialize() override;
    bool m_cursor_locked = false;
};
}// namespace engine::main
