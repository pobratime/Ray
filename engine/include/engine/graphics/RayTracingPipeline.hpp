#pragma once

#include "engine/resources/Model.hpp"

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize();
    void bind_resources(const std::vector<resources::Model *> &models);

    void draw(const resources::Model &model);

private:
    void setup_screen_quad();
};
}// namespace engine::graphics
