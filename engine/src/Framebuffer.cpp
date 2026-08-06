// clang-format off
#include <glad/glad.h>
// clang-format on
#include "engine/graphics/Framebuffer.hpp"
#include "engine/graphics/OpenGL.hpp"

namespace engine::graphics {

void Framebuffer::initialize(const FramebufferSpecs &specs) {
    destroy();
    m_specs = specs;
    CHECKED_GL_CALL(glCreateFramebuffers, 1, &m_fbo);
}

void Framebuffer::bind() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy() {
    if (m_fbo != 0) CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_fbo);
    m_fbo = 0;
}

};// namespace engine::graphics
