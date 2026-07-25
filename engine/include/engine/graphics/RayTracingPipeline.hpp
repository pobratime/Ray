#pragma once

#include "engine/resources/BVHTree.hpp"
#include "engine/resources/Shader.hpp"

namespace engine::graphics {
class RayTracingPipeline {
public:
    void initialize(resources::Shader &shader);
    void upload(resources::BVHTree &tree);
    void render();

private:
    int32_t m_max_texture_buffer_texels = 0;
};
}// namespace engine::graphics
