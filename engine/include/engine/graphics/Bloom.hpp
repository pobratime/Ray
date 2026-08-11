#pragma once

#include "engine/graphics/Framebuffer.hpp"
#include <cstdint>

namespace engine::graphics {

class Bloom {
public:
    struct BloomSettings {
        bool enabled = true;
        float threshold = 1.0f;
        float intensity = 1.0f;
        int blur_passes = 5;
    };

    Bloom() = default;
    ~Bloom() = default;

    void initialize(uint32_t width, uint32_t height);
    void begin(uint32_t width, uint32_t height);
    void end(const BloomSettings &settings);
    void destroy();

private:
    void setup_screen_quad();
    void draw_quad();
    void resize(uint32_t width, uint32_t height);
    void blur_bright(const BloomSettings &settings);
    void composite(float intensity);

    Framebuffer m_scene{};
    Framebuffer m_blur[2]{};
    uint32_t m_blurred = 0;

    uint32_t m_quad_vao = 0;
    uint32_t m_quad_vbo = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    static constexpr uint32_t SCENE_UNIT = 1;
    static constexpr uint32_t BLOOM_UNIT = 2;
};

}// namespace engine::graphics
