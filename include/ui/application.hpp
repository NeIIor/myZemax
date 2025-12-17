#ifndef UI_APPLICATION_HPP
#define UI_APPLICATION_HPP

#include "dr4/window.hpp"
#include "hui/ui.hpp"
#include "cum/manager.hpp"
#include "raytracer/scene.hpp"
#include "raytracer/camera.hpp"
#include "raytracer/raytracer.hpp"
#include <string>

namespace ui {

class MainWindow;

class Application {
public:
    dr4::Window* window;
    hui::UI* ui;
    cum::Manager* pluginManager;
    MainWindow* mainWindow;
    
    raytracer::Scene scene;
    raytracer::Camera camera;
    raytracer::RayTracer* raytracer;
    std::string pluginsDir;

    Application();
    ~Application();

    void Run();
    void ProcessEvents();
    void Update(float deltaTime);
    void Render();
    
private:
    void LoadPlugins();
};

} // namespace ui

#endif // UI_APPLICATION_HPP

