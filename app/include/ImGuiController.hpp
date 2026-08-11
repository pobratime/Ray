//
// Created by kalu on 8/10/26.
//
#pragma once
#include "engine/core/Controller.hpp"
#include "engine/core/Engine.hpp"

namespace engine::main {
class ImGuiController final : public core::Controller {
public:
private:
    void initialize() override;

    void poll_events() override;

    void draw() override;
};
}// namespace engine::main
