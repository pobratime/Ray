#pragma once

#include <cstdint>
namespace engine::graphics {

class Framebuffer {
public:
    struct FramebufferSpecs {
        uint32_t format;
        uint32_t filter;
        uint32_t wrap;
    };

    Framebuffer() = default;
    ~Framebuffer() = default;

    void initialize();
    void bind();
    void unbind();
    void destroy();

private:
    uint32_t m_fbo = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

}// namespace engine::graphics
