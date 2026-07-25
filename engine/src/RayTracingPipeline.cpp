#include "engine/graphics/RayTracingPipeline.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/BVHTree.hpp"
#include "engine/resources/Shader.hpp"
#include "engine/util/Errors.hpp"
#include "glad/glad.h"
#include "spdlog/spdlog.h"

namespace engine::graphics {
void RayTracingPipeline::initialize(resources::Shader &shader) {
    CHECKED_GL_CALL(glGetIntegerv, GL_MAX_TEXTURE_BUFFER_SIZE, &m_max_texture_buffer_texels);
    spdlog::info("GL_MAX_TEXTURE_BUFFER_SIZE = {}", m_max_texture_buffer_texels);
}

void RayTracingPipeline::upload(resources::BVHTree &tree) {
    std::vector<float> nodes = tree.serialize_nodes();
    std::vector<float> primitives = tree.serialize_primitives();

    size_t node_texels = nodes.size() / 4;
    size_t primitive_texels = primitives.size() / 4;

    RG_GUARANTEE(node_texels <= static_cast<size_t>(m_max_texture_buffer_texels),
                 "BVH node buffer needs {} texels but GL_MAX_TEXTURE_BUFFER_SIZE is {}.",
                 node_texels, m_max_texture_buffer_texels);
    RG_GUARANTEE(primitive_texels <= static_cast<size_t>(m_max_texture_buffer_texels),
                 "BVH primitive buffer needs {} texels but GL_MAX_TEXTURE_BUFFER_SIZE is {}.",
                 primitive_texels, m_max_texture_buffer_texels);
}

void RayTracingPipeline::render() {
    // TODO
    // camera params imgui stuff etc etc
}
}// namespace engine::graphics
