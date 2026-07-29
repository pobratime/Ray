#pragma once


namespace engine::main::app {
class Scene {
public:
    Scene() = default;
    ~Scene() = default;
    void initialize();
    void render();

private:
    void setup_screen_quad();
};
}// namespace engine::main::app
