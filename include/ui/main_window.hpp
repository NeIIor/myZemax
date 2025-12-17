#ifndef UI_MAIN_WINDOW_HPP
#define UI_MAIN_WINDOW_HPP

#include "hui/container.hpp"
#include "cum/manager.hpp"
#include <vector>
#include <algorithm>
#include <string>

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
    class AddObjectDialog;
}

namespace ui {

class MainWindow : public hui::Container {
public:
    MainWindow(hui::UI* ui);
    ~MainWindow();
    
    void SetupLayout(raytracer::Scene* scene, raytracer::Camera* camera, raytracer::RayTracer* rt);
    void SetupConnections(raytracer::Scene* scene);
    void SetPluginManager(cum::Manager* manager);
    void SetPluginsDirectory(std::string dir);
    
    PropertiesWindow* GetPropertiesWindow();

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnIdle(hui::IdleEvent& evt) override;

private:
    RayTracerWindow* raytracerWindow;
    ControlPanel* controlPanel;
    ObjectsPanel* objectsPanel;
    PropertiesWindow* propertiesWindow;
    Toolbar* toolbar;
    cum::Manager* pluginManager = nullptr;
    class DrawOverlay* drawOverlay = nullptr;
    AddObjectDialog* addObjectDialog = nullptr;
    std::string pluginsDir;

    
    
    hui::Widget* mouseCapture = nullptr;
    
    std::vector<hui::Widget*> children;
    bool drawMode = false;
    
    void AddChild(hui::Widget* child);
    void BringToFront(hui::Widget* child);
    void ToggleDrawMode();
};

} // namespace ui

#endif // UI_MAIN_WINDOW_HPP