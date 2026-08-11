#pragma once

#include <cstddef>
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
    // Rebuilds the textures for the new size. Everything drawn before is lost.
    void resize(uint32_t width, uint32_t height);
    void destroy();

    // Texture of the i-th format from the specs, so it can be sampled in a shader.
    uint32_t texture(size_t index = 0) const {
        return m_textures[index];
    }

    uint32_t width() const {
        return m_specs.width;
    }

    uint32_t height() const {
        return m_specs.height;
    }

private:
    static int32_t opengl_format(TextureFormat format);

    FramebufferSpecs m_specs{};
    uint32_t m_fbo = 0;
    std::vector<uint32_t> m_textures{};
};

}// namespace engine::graphics
