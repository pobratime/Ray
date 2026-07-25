#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/resources/BVHTree.hpp"
#include "engine/resources/Shader.hpp"

namespace engine::graphics {
void RayTracingPipeline::initialize(resources::Shader &shader) {
}
void RayTracingPipeline::upload(resources::BVHTree &tree) {
    std::vector<float> nodes = tree.serialize_nodes();
    std::vector<float> primitives = tree.serialize_primitives();
}
void RayTracingPipeline::render() {
    // TODO
    // camera params imgui stuff etc etc
}
}// namespace engine::graphics
