#ifndef UI_APPLICATION_HPP
#define UI_APPLICATION_HPP

#include "dr4/window.hpp"
#include "hui/ui.hpp"
#include "cum/manager.hpp"
#include "raytracer/scene.hpp"
#include "raytracer/camera.hpp"
#include "raytracer/raytracer.hpp"

namespace ui {

class Application {
public:
    dr4::Window* window;
    hui::UI* ui;
    cum::Manager* pluginManager;
    
    raytracer::Scene scene;
    raytracer::Camera camera;
    raytracer::RayTracer* raytracer;

    Application();
    ~Application();

    void Run();
    void ProcessEvents();
    void Update(float deltaTime);
    void Render();
    
private:
    void LoadPlugins(); // Загрузка плагинов из папки plugins/
};

} // namespace ui

#endif // UI_APPLICATION_HPP

