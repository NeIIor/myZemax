#ifndef UI_MAIN_WINDOW_HPP
#define UI_MAIN_WINDOW_HPP

#include "hui/container.hpp"
#include "cum/manager.hpp"
#include <vector>

namespace raytracer {
    class Scene;
    class Camera;
    class RayTracer;
    class Object;
}

namespace ui {
    class RayTracerWindow;
    class ControlPanel;
    class ObjectsPanel;
    class Toolbar;
    class PropertiesWindow;
}

namespace ui {

class MainWindow : public hui::Container {
public:
    MainWindow(hui::UI* ui);
    ~MainWindow();
    
    void SetupLayout(raytracer::Scene* scene, raytracer::Camera* camera, raytracer::RayTracer* rt);
    void SetupConnections(raytracer::Scene* scene);
    void SetPluginManager(cum::Manager* manager);
    
    PropertiesWindow* GetPropertiesWindow();

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;

private:
    RayTracerWindow* raytracerWindow;
    ControlPanel* controlPanel;
    ObjectsPanel* objectsPanel;
    PropertiesWindow* propertiesWindow;
    Toolbar* toolbar;
    
    std::vector<hui::Widget*> children;
    
    void AddChild(hui::Widget* child);
};

} // namespace ui

#endif // UI_MAIN_WINDOW_HPP