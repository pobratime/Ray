#pragma once

#include "engine/resources/BVHTree.hpp"

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize();
    // TODO maybe RayTracingPipeline should take a model instead of a finished tree?
    // TODO check with professor
    // maybe i can use the tree for more stuff? -> investigate
    void upload(resources::BVHTree &tree);
    void bind_resources();
    void draw();

private:
    void setup_screen_quad();

    int32_t m_max_texture_buffer_texels = 0;

    uint32_t m_node_buffer = 0;
    uint32_t m_node_texture = 0;

    uint32_t m_primitive_buffer = 0;
    uint32_t m_primitive_texture = 0;

    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
};
}// namespace engine::graphics
