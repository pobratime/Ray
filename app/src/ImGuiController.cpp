//
// Created by kalu on 8/10/26.
//
#include "ImGuiController.hpp"
#include "Scene.hpp"
#include "engine/core/Controller.hpp"
#include "engine/graphics/GraphicsController.hpp"
#include "imgui.h"

namespace engine::main {
void ImGuiController::initialize() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // https://github.com/ocornut/imgui/issues/707#issuecomment-3592676777
    // Catppuccin Mocha Palette
    // --------------------------------------------------------
    const ImVec4 base = ImVec4(0.117f, 0.117f, 0.172f, 1.0f);    // #1e1e2e
    const ImVec4 mantle = ImVec4(0.109f, 0.109f, 0.156f, 1.0f);  // #181825
    const ImVec4 surface0 = ImVec4(0.200f, 0.207f, 0.286f, 1.0f);// #313244
    const ImVec4 surface1 = ImVec4(0.247f, 0.254f, 0.337f, 1.0f);// #3f4056
    const ImVec4 surface2 = ImVec4(0.290f, 0.301f, 0.388f, 1.0f);// #4a4d63
    const ImVec4 overlay0 = ImVec4(0.396f, 0.403f, 0.486f, 1.0f);// #65677c
    const ImVec4 overlay2 = ImVec4(0.576f, 0.584f, 0.654f, 1.0f);// #9399b2
    const ImVec4 text = ImVec4(0.803f, 0.815f, 0.878f, 1.0f);    // #cdd6f4
    const ImVec4 subtext0 = ImVec4(0.639f, 0.658f, 0.764f, 1.0f);// #a3a8c3
    const ImVec4 mauve = ImVec4(0.796f, 0.698f, 0.972f, 1.0f);   // #cba6f7
    const ImVec4 peach = ImVec4(0.980f, 0.709f, 0.572f, 1.0f);   // #fab387
    const ImVec4 yellow = ImVec4(0.980f, 0.913f, 0.596f, 1.0f);  // #f9e2af
    const ImVec4 green = ImVec4(0.650f, 0.890f, 0.631f, 1.0f);   // #a6e3a1
    const ImVec4 teal = ImVec4(0.580f, 0.886f, 0.819f, 1.0f);    // #94e2d5
    const ImVec4 sapphire = ImVec4(0.458f, 0.784f, 0.878f, 1.0f);// #74c7ec
    const ImVec4 blue = ImVec4(0.533f, 0.698f, 0.976f, 1.0f);    // #89b4fa
    const ImVec4 lavender = ImVec4(0.709f, 0.764f, 0.980f, 1.0f);// #b4befe

    // Main window and backgrounds
    colors[ImGuiCol_WindowBg] = base;
    colors[ImGuiCol_ChildBg] = base;
    colors[ImGuiCol_PopupBg] = surface0;
    colors[ImGuiCol_Border] = surface1;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_FrameBg] = surface0;
    colors[ImGuiCol_FrameBgHovered] = surface1;
    colors[ImGuiCol_FrameBgActive] = surface2;
    colors[ImGuiCol_TitleBg] = mantle;
    colors[ImGuiCol_TitleBgActive] = surface0;
    colors[ImGuiCol_TitleBgCollapsed] = mantle;
    colors[ImGuiCol_MenuBarBg] = mantle;
    colors[ImGuiCol_ScrollbarBg] = surface0;
    colors[ImGuiCol_ScrollbarGrab] = surface2;
    colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
    colors[ImGuiCol_ScrollbarGrabActive] = overlay2;
    colors[ImGuiCol_CheckMark] = green;
    colors[ImGuiCol_SliderGrab] = sapphire;
    colors[ImGuiCol_SliderGrabActive] = blue;
    colors[ImGuiCol_Button] = surface0;
    colors[ImGuiCol_ButtonHovered] = surface1;
    colors[ImGuiCol_ButtonActive] = surface2;
    colors[ImGuiCol_Header] = surface0;
    colors[ImGuiCol_HeaderHovered] = surface1;
    colors[ImGuiCol_HeaderActive] = surface2;
    colors[ImGuiCol_Separator] = surface1;
    colors[ImGuiCol_SeparatorHovered] = mauve;
    colors[ImGuiCol_SeparatorActive] = mauve;
    colors[ImGuiCol_ResizeGrip] = surface2;
    colors[ImGuiCol_ResizeGripHovered] = mauve;
    colors[ImGuiCol_ResizeGripActive] = mauve;
    colors[ImGuiCol_Tab] = surface0;
    colors[ImGuiCol_TabHovered] = surface2;
    colors[ImGuiCol_TabActive] = surface1;
    colors[ImGuiCol_TabUnfocused] = surface0;
    colors[ImGuiCol_TabUnfocusedActive] = surface1;
    colors[ImGuiCol_PlotLines] = blue;
    colors[ImGuiCol_PlotLinesHovered] = peach;
    colors[ImGuiCol_PlotHistogram] = teal;
    colors[ImGuiCol_PlotHistogramHovered] = green;
    colors[ImGuiCol_TableHeaderBg] = surface0;
    colors[ImGuiCol_TableBorderStrong] = surface1;
    colors[ImGuiCol_TableBorderLight] = surface0;
    colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = surface2;
    colors[ImGuiCol_DragDropTarget] = yellow;
    colors[ImGuiCol_NavHighlight] = lavender;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);
    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = subtext0;

    // Rounded corners
    style.WindowRounding = 6.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    // Padding and spacing
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.IndentSpacing = 21.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    // Borders
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
}

void ImGuiController::poll_events() {
    const auto platform = engine::core::Controller::get<platform::PlatformController>();
    if (platform->key(platform::KeyId::KEY_F2).state() == platform::Key::State::JustPressed) {
        set_enable(!is_enabled());
    }
}

void ImGuiController::draw() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto scene = engine::core::Controller::get<Scene>();
    auto &s = scene->settings;

    graphics->begin_gui();

    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_FirstUseEver);
    ImGui::Begin("Renderer");

    const ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("%.1f FPS", io.Framerate);
    ImGui::SameLine();
    ImGui::TextDisabled("(%.2f ms)", 1000.0f / io.Framerate);

    ImGui::Spacing();
    ImGui::PushItemWidth(-110.0f);

    ImGui::SeparatorText("Lighting");
    ImGui::SliderInt("Light samples", &s.light_samples, 0, 16);
    ImGui::BeginDisabled(s.light_samples == 0);
    ImGui::SliderFloat("Power", &s.light_power, 0.0f, 10.0f, "%.2f");
    ImGui::EndDisabled();
    ImGui::SliderFloat("Ambient", &s.ambient, 0.0f, 1.0f, "%.2f");

    ImGui::SeparatorText("Reflections");
    ImGui::SliderInt("Bounces", &s.reflection_count, 0, 16);
    ImGui::BeginDisabled(s.reflection_count == 0);
    ImGui::SliderFloat("Min metalness", &s.min_reflection, 0.0f, 1.0f, "%.2f");
    ImGui::EndDisabled();

    ImGui::SeparatorText("Features");
    ImGui::Checkbox("Textures", &s.use_textures);

    ImGui::PopItemWidth();
    ImGui::End();

    graphics->end_gui();
}
}// namespace engine::main
