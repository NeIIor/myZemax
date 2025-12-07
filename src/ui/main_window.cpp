#include "ui/main_window.hpp"

#include "ui/raytracer_window.hpp"
#include "ui/control_panel.hpp"
#include "ui/objects_panel.hpp"
#include "ui/toolbar.hpp"
#include "ui/properties_window.hpp"
#include "raytracer/raytracer.hpp"

namespace ui {

MainWindow::MainWindow(hui::UI* ui) : Container(ui),
    raytracerWindow(nullptr),
    controlPanel(nullptr),
    objectsPanel(nullptr),
    propertiesWindow(nullptr),
    toolbar(nullptr) {
}

MainWindow::~MainWindow() {
    for (auto* child : children) {
        UnbecomeParentOf(child);
    }
}

void MainWindow::AddChild(hui::Widget* child) {
    if (child) {
        BecomeParentOf(child);
        children.push_back(child);
    }
}

PropertiesWindow* MainWindow::GetPropertiesWindow() {
    return propertiesWindow;
}

hui::EventResult MainWindow::PropagateToChildren(hui::Event& event) {
    hui::EventResult result = hui::EventResult::UNHANDLED;
    
    for (auto* child : children) {
        if (child) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) {
                result = hui::EventResult::HANDLED;
            }
        }
    }
    
    return result;
}

void MainWindow::SetupLayout(raytracer::Scene* scene, raytracer::Camera* camera, raytracer::RayTracer* rt) {
    raytracerWindow = new RayTracerWindow(GetUI(), scene, camera);
    raytracerWindow->SetRayTracer(rt);
    
    controlPanel = new ControlPanel(GetUI(), camera);
    objectsPanel = new ObjectsPanel(GetUI(), scene);
    propertiesWindow = new PropertiesWindow(GetUI());
    toolbar = new Toolbar(GetUI(), nullptr);
    
    toolbar->SetPos(0, 0);
    toolbar->SetSize(GetSize().x, 30);
    
    raytracerWindow->SetPos(200, 30);
    raytracerWindow->SetSize(GetSize().x - 200, GetSize().y - 30);
    
    controlPanel->SetPos(0, 30);
    controlPanel->SetSize(200, 200);
    
    objectsPanel->SetPos(0, 230);
    objectsPanel->SetSize(200, GetSize().y - 230 - 300);
    
    propertiesWindow->SetPos(0, GetSize().y - 300);
    propertiesWindow->SetSize(200, 300);
    
    AddChild(toolbar);
    AddChild(raytracerWindow);
    AddChild(controlPanel);
    AddChild(objectsPanel);
    AddChild(propertiesWindow);
    
    SetupConnections(scene);
}

void MainWindow::SetupConnections(raytracer::Scene* scene) {
    objectsPanel->SetOnObjectSelected([this](raytracer::Object* obj) {
        if (obj) {
            raytracerWindow->SetSelectedObject(obj);
            propertiesWindow->SetObject(obj);
        }
    });
    
    auto pasteHandler = [this, scene]() {
        if (propertiesWindow && propertiesWindow->HasCopiedObject()) {
            propertiesWindow->PasteObject(scene);
            objectsPanel->RefreshList();
            raytracerWindow->MarkDirty();
        }
    };
    
    propertiesWindow->SetOnPasteRequest(pasteHandler);
    raytracerWindow->SetOnPasteRequest(pasteHandler);
}

void MainWindow::SetPluginManager(cum::Manager* manager) {
    if (toolbar) {
        toolbar->SetManager(manager);
    }
}

} // namespace ui