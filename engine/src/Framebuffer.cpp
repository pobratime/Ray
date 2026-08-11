// clang-format off
#include <glad/glad.h>
// clang-format on
#include "engine/graphics/Framebuffer.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/util/Errors.hpp"
#include <cstdint>
#include <vector>

namespace engine::graphics {

void Framebuffer::initialize(const FramebufferSpecs &specs) {
    destroy();
    m_specs = specs;
    RG_GUARANTEE(!m_specs.formats.empty(), "Framebuffer needs at least one format.");
    RG_GUARANTEE(m_specs.width > 0 && m_specs.height > 0, "Framebuffer size can't be 0.");

    CHECKED_GL_CALL(glCreateFramebuffers, 1, &m_fbo);

    // one texture per format, attached in the same order
    m_textures.resize(m_specs.formats.size());
    CHECKED_GL_CALL(glCreateTextures, GL_TEXTURE_2D, static_cast<int32_t>(m_textures.size()), m_textures.data());

    std::vector<uint32_t> attachments{};
    attachments.reserve(m_textures.size());
    for (size_t i = 0; i < m_textures.size(); i++) {
        CHECKED_GL_CALL(glTextureStorage2D,
                        m_textures[i],
                        1,
                        opengl_format(m_specs.formats[i]),
                        static_cast<int32_t>(m_specs.width),
                        static_cast<int32_t>(m_specs.height));

        CHECKED_GL_CALL(glTextureParameteri, m_textures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTextureParameteri, m_textures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // clamp so sampling the edge doesn't wrap around to the other side
        CHECKED_GL_CALL(glTextureParameteri, m_textures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTextureParameteri, m_textures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        const uint32_t attachment = GL_COLOR_ATTACHMENT0 + static_cast<uint32_t>(i);
        CHECKED_GL_CALL(glNamedFramebufferTexture, m_fbo, attachment, m_textures[i], 0);
        attachments.push_back(attachment);
    }
    // without this only the first attachment gets written to
    CHECKED_GL_CALL(glNamedFramebufferDrawBuffers,
                    m_fbo,
                    static_cast<int32_t>(attachments.size()),
                    attachments.data());

    const uint32_t status = CHECKED_GL_CALL(glCheckNamedFramebufferStatus, m_fbo, GL_FRAMEBUFFER);
    RG_GUARANTEE(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete, status {}.", status);
}

void Framebuffer::bind() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(const uint32_t width, const uint32_t height) {
    if (width == m_specs.width && height == m_specs.height) {
        return;
    }
    // textures have a fixed size once created, so just build everything again
    FramebufferSpecs specs = m_specs;
    specs.width = width;
    specs.height = height;
    initialize(specs);
}

void Framebuffer::destroy() {
    if (!m_textures.empty()) {
        CHECKED_GL_CALL(glDeleteTextures, static_cast<int32_t>(m_textures.size()), m_textures.data());
        m_textures.clear();
    }
    if (m_fbo != 0) CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_fbo);
    m_fbo = 0;
}

int32_t Framebuffer::opengl_format(const TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::RGBA16F: return GL_RGBA16F;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        default: RG_SHOULD_NOT_REACH_HERE("Unknown TextureFormat");
    }
}

};// namespace engine::graphics
