#ifndef UI_MAIN_WINDOW_HPP
#define UI_MAIN_WINDOW_HPP

#include "hui/container.hpp"
#include "ui/raytracer_window.hpp"
#include "ui/control_panel.hpp"
#include "ui/objects_panel.hpp"
#include "ui/toolbar.hpp"
#include "cum/manager.hpp"

namespace ui {

class MainWindow : public hui::Container {
public:
    MainWindow(hui::UI* ui);
    
    void SetupLayout(raytracer::Scene* scene, raytracer::Camera* camera, raytracer::RayTracer* rt);
    void SetupConnections(raytracer::Scene* scene);
    void SetPluginManager(cum::Manager* manager);
    
    PropertiesWindow* GetPropertiesWindow() { return propertiesWindow; }
    
private:
    RayTracerWindow* raytracerWindow;
    ControlPanel* controlPanel;
    ObjectsPanel* objectsPanel;
    PropertiesWindow* propertiesWindow;
    Toolbar* toolbar;
};

} // namespace ui

#endif // UI_MAIN_WINDOW_HPP

