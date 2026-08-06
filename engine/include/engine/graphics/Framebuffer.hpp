#pragma once

#include <cstdint>
#include <vector>
namespace engine::graphics {

class Framebuffer {
public:
    enum class TextureFormat {
        RGBA8,
        RGBA16F,
        RGBA32F
    };
    struct FramebufferSpecs {
        uint32_t width;
        uint32_t height;
        std::vector<TextureFormat> formats;
    };

    Framebuffer() = default;
    ~Framebuffer() = default;

    void initialize(const FramebufferSpecs &specs);
    void bind();
    void unbind();
    void destroy();

private:
    FramebufferSpecs m_specs;
    uint32_t m_fbo = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

}// namespace engine::graphics
