#include "ui/main_window.hpp"

#include "ui/raytracer_window.hpp"
#include "ui/control_panel.hpp"
#include "ui/objects_panel.hpp"
#include "ui/toolbar.hpp"
#include "ui/properties_window.hpp"
#include "ui/add_object_dialog.hpp"
#include "ui/draw_overlay.hpp"
#include "raytracer/raytracer.hpp"
#include "raytracer/objects.hpp"
#include "hui/ui.hpp"
#include "dr4/keycodes.hpp"
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include "cum/ifc/pp.hpp"

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

void MainWindow::SetPluginsDirectory(std::string dir) {
    pluginsDir = std::move(dir);
    if (toolbar) {
        toolbar->SetPluginsDirectory(pluginsDir);
    }
}

hui::EventResult MainWindow::PropagateToChildren(hui::Event& event) {

    if (auto* k = dynamic_cast<hui::KeyEvent*>(&event)) {
        if (k->pressed && k->key == dr4::KEYCODE_F) {
            ToggleDrawMode();
            return hui::EventResult::HANDLED;
        }
    }


    if (drawMode && drawOverlay) {
        return event.Apply(*drawOverlay);
    }


    if (dynamic_cast<hui::KeyEvent*>(&event)) {
        static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
        for (int idx = static_cast<int>(children.size()) - 1; idx >= 0; --idx) {
            auto* child = children[idx];
            if (!child) continue;
            auto res = event.Apply(*child);
            if (debugInput) {
                std::cout << "[ui] MainWindow key -> child idx=" << idx << " handled=" << (res == hui::EventResult::HANDLED) << "\n";
            }
            if (res == hui::EventResult::HANDLED) {
                return hui::EventResult::HANDLED;
            }
        }
        return hui::EventResult::UNHANDLED;
    }


    
    
    if (mouseCapture) {
        if (auto* m = dynamic_cast<hui::MouseMoveEvent*>(&event)) {
            dr4::Vec2f savedPos = m->pos;
            m->pos -= mouseCapture->GetPos();
            (void)event.Apply(*mouseCapture);
            m->pos = savedPos;
            return hui::EventResult::HANDLED;
        }
        if (auto* m = dynamic_cast<hui::MouseButtonEvent*>(&event)) {
            
            if (m->button == dr4::MouseButtonType::LEFT && !m->pressed) {
                dr4::Vec2f savedPos = m->pos;
                m->pos -= mouseCapture->GetPos();
                (void)event.Apply(*mouseCapture);
                m->pos = savedPos;
                mouseCapture = nullptr;
                return hui::EventResult::HANDLED;
            }
        }
    }

    for (int idx = static_cast<int>(children.size()) - 1; idx >= 0; --idx) {
        auto* child = children[idx];
        if (!child) continue;

        dr4::Vec2f savedPos{};
        bool hasPos = false;
        bool inside = true;

        if (auto* m = dynamic_cast<hui::MouseButtonEvent*>(&event)) {
            savedPos = m->pos;
            hasPos = true;
            m->pos -= child->GetPos();
            inside = (m->pos.x >= 0 && m->pos.y >= 0 &&
                      m->pos.x <= child->GetSize().x && m->pos.y <= child->GetSize().y);
        } else if (auto* m = dynamic_cast<hui::MouseMoveEvent*>(&event)) {
            savedPos = m->pos;
            hasPos = true;
            m->pos -= child->GetPos();
            inside = (m->pos.x >= 0 && m->pos.y >= 0 &&
                      m->pos.x <= child->GetSize().x && m->pos.y <= child->GetSize().y);
        } else if (auto* m = dynamic_cast<hui::MouseWheelEvent*>(&event)) {
            savedPos = m->pos;
            hasPos = true;
            m->pos -= child->GetPos();
            inside = (m->pos.x >= 0 && m->pos.y >= 0 &&
                      m->pos.x <= child->GetSize().x && m->pos.y <= child->GetSize().y);
        }

        if (hasPos && !inside) {

            if (auto* m = dynamic_cast<hui::MouseButtonEvent*>(&event)) m->pos = savedPos;
            else if (auto* m = dynamic_cast<hui::MouseMoveEvent*>(&event)) m->pos = savedPos;
            else if (auto* m = dynamic_cast<hui::MouseWheelEvent*>(&event)) m->pos = savedPos;
            continue;
        }

        auto applied = event.Apply(*child);
        if (applied == hui::EventResult::HANDLED) {
            if (hasPos) {
                if (auto* m = dynamic_cast<hui::MouseButtonEvent*>(&event)) m->pos = savedPos;
                else if (auto* m = dynamic_cast<hui::MouseMoveEvent*>(&event)) m->pos = savedPos;
                else if (auto* m = dynamic_cast<hui::MouseWheelEvent*>(&event)) m->pos = savedPos;
            }
            BringToFront(child);
            
            if (auto* mb = dynamic_cast<hui::MouseButtonEvent*>(&event)) {
                if (mb->button == dr4::MouseButtonType::LEFT && mb->pressed) {
                    mouseCapture = child;
                }
            }
            return hui::EventResult::HANDLED;
        }

        if (hasPos) {
            if (auto* m = dynamic_cast<hui::MouseButtonEvent*>(&event)) m->pos = savedPos;
            else if (auto* m = dynamic_cast<hui::MouseMoveEvent*>(&event)) m->pos = savedPos;
            else if (auto* m = dynamic_cast<hui::MouseWheelEvent*>(&event)) m->pos = savedPos;
        }
    }
    return hui::EventResult::UNHANDLED;
}

void MainWindow::BringToFront(hui::Widget* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end() && std::next(it) != children.end()) {
        hui::Widget* ptr = *it;
        children.erase(it);
        children.push_back(ptr);
    }
}

void MainWindow::Redraw() const {
    dr4::Texture& texture = GetTexture();
    texture.Clear(dr4::Color(45, 45, 45));
    
    for (auto* child : children) {
        if (child) {
            child->DrawOn(texture);
        }
    }
}

void MainWindow::SetupLayout(raytracer::Scene* scene, raytracer::Camera* camera, raytracer::RayTracer* rt) {
    raytracerWindow = new RayTracerWindow(GetUI(), scene, camera);
    raytracerWindow->SetRayTracer(rt);
    
    controlPanel = new ControlPanel(GetUI(), camera);
    objectsPanel = new ObjectsPanel(GetUI(), scene);
    propertiesWindow = new PropertiesWindow(GetUI());
    toolbar = new Toolbar(GetUI(), nullptr);
    drawOverlay = new DrawOverlay(GetUI());
    addObjectDialog = new AddObjectDialog(GetUI());

    const float topBarHeight = 36.0f;
    const float margin = 12.0f;
    const float gap = 12.0f;
    const float leftDockWidth = 280.0f;
    const float rightDockWidth = 360.0f;
    const float winW = GetSize().x;
    const float winH = GetSize().y;

    const float contentX = margin;
    const float contentY = topBarHeight + margin;
    const float contentW = std::max(0.0f, winW - 2.0f * margin);
    const float contentH = std::max(0.0f, winH - topBarHeight - 2.0f * margin);

    const float leftX = contentX;
    const float rightX = contentX + contentW - rightDockWidth;
    const float viewportX = leftX + leftDockWidth + gap;
    const float viewportW = std::max(0.0f, rightX - gap - viewportX);
    
    
    toolbar->SetPos(0, 0);
    toolbar->SetSize(winW, winH);
    
    raytracerWindow->SetPos(viewportX, contentY);
    raytracerWindow->SetSize(viewportW, contentH);
    
    const float controlH = 280.0f;
    const float objectsH = std::max(60.0f, contentH - controlH - gap);

    controlPanel->SetPos(leftX, contentY);
    controlPanel->SetSize(leftDockWidth, controlH);
    
    objectsPanel->SetPos(leftX, contentY + controlH + gap);
    objectsPanel->SetSize(leftDockWidth, objectsH);
    
    propertiesWindow->SetPos(rightX, contentY);
    propertiesWindow->SetSize(rightDockWidth, contentH);
    if (drawOverlay) {
        drawOverlay->SetPos(0, 0);
        drawOverlay->SetSize(winW, winH);
    }
    if (addObjectDialog) {
        addObjectDialog->SetPos(winW * 0.5f - 130.0f, winH * 0.5f - 130.0f);
        addObjectDialog->Hide();
    }
    

    children.clear();
    AddChild(toolbar);
    AddChild(controlPanel);
    AddChild(objectsPanel);
    AddChild(raytracerWindow);
    AddChild(propertiesWindow);
    AddChild(drawOverlay);
    AddChild(addObjectDialog);
    
    SetupConnections(scene);


    GetUI()->ReportFocus(controlPanel);
}

void MainWindow::SetupConnections(raytracer::Scene* scene) {
    objectsPanel->SetOnObjectSelected([this](raytracer::Object* obj) {
        if (obj) {
            raytracerWindow->SetSelectedObject(obj);
            propertiesWindow->SetObject(obj);
            raytracerWindow->MarkDirty();
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

    raytracerWindow->SetOnObjectSelected([this](raytracer::Object* obj) {
        if (!obj) return;
        objectsPanel->SetSelectedObject(obj);
        propertiesWindow->SetObject(obj);
        raytracerWindow->MarkDirty();
    });

    propertiesWindow->SetOnObjectChanged([this]() {
        objectsPanel->RefreshList();
        raytracerWindow->MarkDirty();
    });

    propertiesWindow->SetOnObjectCommitted([this](raytracer::Object* obj) {
        if (!obj) return;
        objectsPanel->SetSelectedObject(obj);
        raytracerWindow->SetSelectedObject(obj);
        objectsPanel->RefreshList();
        raytracerWindow->MarkDirty();
    });

    objectsPanel->SetOnSceneChanged([this]() {
        raytracerWindow->MarkDirty();
        objectsPanel->RefreshList();
    });

    controlPanel->SetOnCameraChanged([this]() {
        raytracerWindow->MarkDirty();
    });

    objectsPanel->SetOnAddRequested([this]() {
        if (!addObjectDialog) return;
        addObjectDialog->Show();
        BringToFront(addObjectDialog);
        ForceRedraw();
    });

    if (addObjectDialog) {
        addObjectDialog->SetOnCancel([this]() {
            if (addObjectDialog) addObjectDialog->Hide();
            ForceRedraw();
        });
        addObjectDialog->SetOnOk([this, scene](int kind) {
            if (!propertiesWindow) return;
            if (!scene) return;

            auto makeUnique = [scene](const std::string& base) {
                int counter = 1;
                std::string candidate = base;
                while (scene->FindObjectByName(candidate)) {
                    candidate = base + " " + std::to_string(counter++);
                }
                return candidate;
            };

            std::unique_ptr<raytracer::Object> created;
            switch (kind) {
                case 0: {
                    auto obj = std::make_unique<raytracer::Sphere>(1.0f, makeUnique("Sphere"));
                    obj->position = raytracer::Vec3(0, 0, 0);
                    created = std::move(obj);
                    break;
                }
                case 1: {
                    auto obj = std::make_unique<raytracer::Plane>(raytracer::Vec3(0, 1, 0), makeUnique("Plane"));
                    obj->position = raytracer::Vec3(0, -1, 0);
                    created = std::move(obj);
                    break;
                }
                case 6: {
                    auto obj = std::make_unique<raytracer::RectPlane>(3.0f, 3.0f, raytracer::Vec3(0, 1, 0), makeUnique("BoundedPlane"));
                    obj->position = raytracer::Vec3(0, -1, 0);
                    obj->color = dr4::Color(160, 220, 180);
                    obj->reflectivity = 0.10f;
                    created = std::move(obj);
                    break;
                }
                case 2: {
                    auto obj = std::make_unique<raytracer::Disk>(1.0f, raytracer::Vec3(0, 1, 0), makeUnique("Disk"));
                    obj->position = raytracer::Vec3(0, 0, 0);
                    created = std::move(obj);
                    break;
                }
                case 3: {
                    auto obj = std::make_unique<raytracer::Prism>(raytracer::Vec3(1, 1, 1), makeUnique("Prism"));
                    created = std::move(obj);
                    break;
                }
                case 4: {
                    auto obj = std::make_unique<raytracer::Pyramid>(1.2f, 1.2f, makeUnique("Pyramid"));
                    created = std::move(obj);
                    break;
                }
                case 5: {
                    auto obj = std::make_unique<raytracer::Sphere>(0.4f, makeUnique("Light"));
                    obj->isLightSource = true;
                    obj->color = dr4::Color(255, 255, 255);
                    obj->position = raytracer::Vec3(2.0f, 2.0f, 2.0f);
                    created = std::move(obj);
                    break;
                }
                default:
                    return;
            }

            propertiesWindow->StartDraft(scene, std::move(created));
            BringToFront(propertiesWindow);
            ForceRedraw();
        });
    }
}

void MainWindow::SetPluginManager(cum::Manager* manager) {
    pluginManager = manager;
    if (toolbar) {
        toolbar->SetManager(manager);
        if (!pluginsDir.empty()) {
            toolbar->SetPluginsDirectory(pluginsDir);
        }
    }
    if (drawOverlay) {
        drawOverlay->SetPluginManager(manager);
    }

    
    if (pluginManager) {
        auto toolsPlugins = pluginManager->GetAllOfType<cum::PPToolPlugin>();
        if (!toolsPlugins.empty()) {
            if (drawOverlay) {
                drawOverlay->SetToolsPluginIdFilter(std::string(toolsPlugins[0]->GetIdentifier()));
            }
            if (toolbar) {
                toolbar->SetActiveToolsPluginLabel(std::string(toolsPlugins[0]->GetName()));
            }
        }
    }

    if (toolbar) {
        toolbar->SetOnToolsPluginSelected([this](const std::string& soPath) {
            if (!pluginManager) return;
            try {
                cum::Plugin* p = pluginManager->LoadFromFile(soPath);
                if (!p) return;

                
                std::unordered_map<std::string_view, cum::Plugin*> byId;
                for (const auto& up : pluginManager->GetAll()) {
                    byId[up->GetIdentifier()] = up.get();
                }

                for (auto& dep : p->GetDependencies()) {
                    if (!byId.count(dep)) {
                        std::cerr << "Plugin dependency error: missing " << dep << "\n";
                        return;
                    }
                }
                for (auto& conf : p->GetConflicts()) {
                    if (byId.count(conf)) {
                        std::cerr << "Plugin conflict: " << p->GetIdentifier() << " conflicts with " << conf << "\n";
                        return;
                    }
                }
                
                for (const auto& up : pluginManager->GetAll()) {
                    for (auto& conf : up->GetConflicts()) {
                        if (conf == p->GetIdentifier()) {
                            std::cerr << "Plugin conflict: " << up->GetIdentifier() << " conflicts with " << p->GetIdentifier() << "\n";
                            return;
                        }
                    }
                }

                p->AfterLoad();

                
                if (drawOverlay) {
                    drawOverlay->SetToolsPluginIdFilter(std::string(p->GetIdentifier()));
                }
                
                if (toolbar) {
                    toolbar->SetActiveToolsPluginLabel(std::string(p->GetName()));
                }
                ForceRedraw();
            } catch (const cum::Manager::LoadError& e) {
                std::cerr << "Failed to load plugin " << soPath << ": " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Failed to load plugin " << soPath << ": unknown error\n";
            }
        });
    }
}

hui::EventResult MainWindow::OnIdle(hui::IdleEvent& evt) {
    if (drawMode) {
        if (drawOverlay) {
            return drawOverlay->OnIdle(evt);
        }
        return hui::EventResult::UNHANDLED;
    }
    return Container::OnIdle(evt);
}

void MainWindow::ToggleDrawMode() {
    drawMode = !drawMode;
    if (drawMode && drawOverlay && pluginManager) {
        drawOverlay->EnsureLoaded();
        drawOverlay->SetPos(0, 0);
        drawOverlay->SetSize(GetSize());
        drawOverlay->ToggleVisible(true);

        BringToFront(drawOverlay);
        drawOverlay->ForceRedraw();
    } else if (drawOverlay) {
        drawOverlay->ToggleVisible(false);
        drawOverlay->ForceRedraw();
    }
    if (toolbar) {
        toolbar->SetDrawMode(drawMode);
        toolbar->ForceRedraw();
    }
    ForceRedraw();
}

} // namespace ui