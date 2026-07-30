#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/Model.hpp"
#include <cstdint>
#include <vector>

namespace engine::graphics {
void RayTracingPipeline::initialize() {
}

void RayTracingPipeline::bind_resources(const std::vector<resources::Model *> &models) {
    for (unsigned int i = 0; i < static_cast<unsigned int>(models.size()); i += 2) {
        models[i]->bind(i, i + 1);
    }
}

void RayTracingPipeline::draw(const resources::Model &model) {
}

void RayTracingPipeline::setup_screen_quad() {
}

}// namespace engine::graphics
