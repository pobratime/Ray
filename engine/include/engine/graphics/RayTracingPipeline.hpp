#pragma once

#include "engine/resources/Model.hpp"

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize();
    // TODO maybe RayTracingPipeline should take a model instead of a finished tree?
    // TODO check with professor
    // maybe i can use the tree for more stuff? -> investigate
    void upload(const resources::Model &model);
    void bind_resources();
    void draw();

private:
    void setup_screen_quad();
};
}// namespace engine::graphics
