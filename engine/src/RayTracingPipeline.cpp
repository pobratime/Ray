#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/Model.hpp"

namespace engine::graphics {
void RayTracingPipeline::initialize() {
}

void RayTracingPipeline::upload(const resources::Model &model) {
    model.bvh();
}

void RayTracingPipeline::bind_resources() {
}

void RayTracingPipeline::draw() {
}

void RayTracingPipeline::setup_screen_quad() {
}

}// namespace engine::graphics
