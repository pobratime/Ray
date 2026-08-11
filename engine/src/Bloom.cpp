// clang-format off
#include <glad/glad.h>
// clang-format on
#include "engine/graphics/Bloom.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/OpenGL.hpp"
#include "engine/resources/ResourcesController.hpp"
#include <algorithm>
#include <cstdint>

namespace engine::graphics {

static uint32_t half_size(const uint32_t size) {
    return std::max(size / 2u, 1u);
}

void Bloom::initialize(const uint32_t width, const uint32_t height) {
    setup_screen_quad();

    m_width = width;
    m_height = height;
    m_scene.initialize({.width = m_width,
                        .height = m_height,
                        .formats = {Framebuffer::TextureFormat::RGBA16F}});

    for (auto &blur: m_blur) {
        blur.initialize({.width = half_size(m_width),
                         .height = half_size(m_height),
                         .formats = {Framebuffer::TextureFormat::RGBA16F}});
        blur.bind();
        CHECKED_GL_CALL(glClearColor, 0.0f, 0.0f, 0.0f, 1.0f);
        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT);
        blur.unbind();
    }
}

void Bloom::begin(const uint32_t width, const uint32_t height) {
    resize(width, height);
    m_scene.bind();
    CHECKED_GL_CALL(glViewport, 0, 0, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height));
    CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT);
}

void Bloom::end(const BloomSettings &settings) {
    m_scene.unbind();
    const bool do_bloom = settings.enabled && settings.blur_passes > 0;
    if (do_bloom) {
        blur_bright(settings);
    }
    composite(do_bloom ? settings.intensity : 0.0f);
}

void Bloom::resize(const uint32_t width, const uint32_t height) {
    if (width == m_width && height == m_height) {
        return;
    }
    m_width = width;
    m_height = height;
    m_scene.resize(m_width, m_height);
    for (auto &blur: m_blur) {
        blur.resize(half_size(m_width), half_size(m_height));
    }
}

void Bloom::blur_bright(const BloomSettings &settings) {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    CHECKED_GL_CALL(glViewport, 0, 0,
                    static_cast<int32_t>(half_size(m_width)),
                    static_cast<int32_t>(half_size(m_height)));

    const auto extract = res_con->shader("bloom_extract");
    extract->use();
    extract->set_int("u_scene", SCENE_UNIT);
    extract->set_float("u_threshold", settings.threshold);
    CHECKED_GL_CALL(glBindTextureUnit, SCENE_UNIT, m_scene.texture());
    m_blur[0].bind();
    draw_quad();

    const auto blur = res_con->shader("bloom_blur");
    blur->use();
    blur->set_int("u_image", SCENE_UNIT);
    uint32_t source = 0;
    bool horizontal = true;
    for (int i = 0; i < settings.blur_passes * 2; i++) {
        m_blur[1 - source].bind();
        blur->set_bool("u_horizontal", horizontal);
        CHECKED_GL_CALL(glBindTextureUnit, SCENE_UNIT, m_blur[source].texture());
        draw_quad();
        source = 1 - source;
        horizontal = !horizontal;
    }
    m_blurred = source;
}

void Bloom::composite(const float intensity) {
    const auto res_con = engine::core::Controller::get<resources::ResourcesController>();
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    CHECKED_GL_CALL(glViewport, 0, 0, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height));

    const auto shader = res_con->shader("bloom_composite");
    shader->use();
    shader->set_int("u_scene", SCENE_UNIT);
    shader->set_int("u_bloom", BLOOM_UNIT);
    shader->set_float("u_intensity", intensity);
    CHECKED_GL_CALL(glBindTextureUnit, SCENE_UNIT, m_scene.texture());
    CHECKED_GL_CALL(glBindTextureUnit, BLOOM_UNIT, m_blur[m_blurred].texture());
    draw_quad();
}

void Bloom::draw_quad() {
    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 6);
    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void Bloom::setup_screen_quad() {
    static constexpr float quad_vertices[] = {
            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            1.0f,
            0.0f,

            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
    };

    CHECKED_GL_CALL(glGenVertexArrays, 1, &m_quad_vao);
    CHECKED_GL_CALL(glGenBuffers, 1, &m_quad_vbo);

    CHECKED_GL_CALL(glBindVertexArray, m_quad_vao);
    CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_quad_vbo);
    CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
    CHECKED_GL_CALL(glVertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                    static_cast<const void *>(nullptr));

    CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
    CHECKED_GL_CALL(glVertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                    reinterpret_cast<const void *>(2 * sizeof(float)));

    CHECKED_GL_CALL(glBindVertexArray, 0);
}

void Bloom::destroy() {
    m_scene.destroy();
    for (auto &blur: m_blur) {
        blur.destroy();
    }
    if (m_quad_vao != 0)
        CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_quad_vao);
    m_quad_vao = 0;
    if (m_quad_vbo != 0)
        CHECKED_GL_CALL(glDeleteBuffers, 1, &m_quad_vbo);
    m_quad_vbo = 0;
}

}// namespace engine::graphics
