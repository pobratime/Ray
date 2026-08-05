#pragma once

#include <cstdint>
namespace engine::graphics {

class Framebuffer {
public:
    void bind();
    void unbind();


private:
    uint32_t m_fbo = 0;
};

}// namespace engine::graphics
