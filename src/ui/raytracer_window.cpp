#include "ui/raytracer_window.hpp"
#include "raytracer/objects.hpp"
#include "dr4/keycodes.hpp"
#include <array>
#include <cmath>
#include <algorithm>
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include <cstdlib>
#include <iostream>

namespace ui {

RayTracerWindow::RayTracerWindow(hui::UI* ui, raytracer::Scene* scene_, raytracer::Camera* camera_)
    : Widget(ui), scene(scene_), camera(camera_), raytracer(nullptr) {
    SetSize(800, 600);
}

void RayTracerWindow::MarkDirty() {
    static bool debugRender = std::getenv("MYZEMAX_DEBUG_RENDER") != nullptr;
    needsRender = true;
    renderDelayFrames = 0;
    if (debugRender) {
        std::cout << "[render] MarkDirty -> needsRender=1\n";
    }
    ForceRedraw();
}

void RayTracerWindow::SetSelectedObject(const raytracer::Object* obj) {
    selectedObject = const_cast<raytracer::Object*>(obj);
    ForceRedraw();
}

void RayTracerWindow::Redraw() const {
    static bool debugRender = std::getenv("MYZEMAX_DEBUG_RENDER") != nullptr;
    dr4::Texture& texture = GetTexture();
    const dr4::Color bgViewport(22, 22, 28);
    const dr4::Color bgHeader(76, 76, 76);
    const dr4::Color borderHeader(96, 96, 96);
    const dr4::Color textMain(235, 235, 235);

    texture.Clear(isCollapsed ? dr4::Color(0, 0, 0, 0) : bgViewport);

    float viewportHeight = std::max(1.0f, GetSize().y - titleBarHeight);
    float viewportWidth = GetSize().x;


    auto* bar = GetUI()->GetWindow()->CreateRectangle();
    bar->SetPos(dr4::Vec2f(0, 0));
    bar->SetSize(dr4::Vec2f(GetSize().x, titleBarHeight));
    bar->SetFillColor(bgHeader);
    bar->SetBorderColor(borderHeader);
    bar->SetBorderThickness(1.0f);
    texture.Draw(*bar);

    auto* titleText = GetUI()->GetWindow()->CreateText();
    titleText->SetText("Ray Tracer");
    titleText->SetPos(dr4::Vec2f(10, 6));
    titleText->SetFontSize(14);
    titleText->SetColor(textMain);
    texture.Draw(*titleText);

    auto* arrowText = GetUI()->GetWindow()->CreateText();
    arrowText->SetText(isCollapsed ? ">" : "v");
    arrowText->SetPos(dr4::Vec2f(GetSize().x - 20, 6));
    arrowText->SetFontSize(14);
    arrowText->SetColor(textMain);
    texture.Draw(*arrowText);

    if (isCollapsed) {
        return;
    }

    if (!raytracer || !scene || !camera) {
        return;
    }


    camera->aspectRatio = viewportWidth / viewportHeight;
    
    if (!renderImage && GetUI() && GetUI()->GetWindow()) {
        renderImage = GetUI()->GetWindow()->CreateImage();
        renderImage->SetSize(dr4::Vec2f(viewportWidth, viewportHeight));
        if (debugRender) {
            std::cout << "[render] create renderImage " << viewportWidth << "x" << viewportHeight << "\n";
        }
    } else if (renderImage) {
        dr4::Vec2f curSize = renderImage->GetSize();
        if (curSize.x != viewportWidth || curSize.y != viewportHeight) {
            renderImage->SetSize(dr4::Vec2f(viewportWidth, viewportHeight));
            needsRender = true;
            if (debugRender) {
                std::cout << "[render] resize renderImage to " << viewportWidth << "x" << viewportHeight << "\n";
            }
        }
    }


    if (needsRender && raytracer && renderImage) {
        raytracer->Render(renderImage);
        needsRender = false;
        if (debugRender) {
            std::cout << "[render] rendered frame\n";
        }
    } else if (needsRender && debugRender && !renderImage) {
        std::cout << "[render] cannot render: renderImage is null\n";
    }


    if (renderImage) {
        renderImage->SetPos(dr4::Vec2f(0, titleBarHeight));
        texture.Draw(*renderImage);
    }
    
    if (selectedObject) {
        DrawSelectionBox(texture, selectedObject);
    }
}

void RayTracerWindow::DrawSelectionBox(dr4::Texture& texture, raytracer::Object* obj) const {
    raytracer::Vec3 min, max;
    obj->GetBoundingBox(min, max);

    const std::array<raytracer::Vec3, 8> corners = {{
        {min.x, min.y, min.z}, {max.x, min.y, min.z}, {min.x, max.y, min.z}, {max.x, max.y, min.z},
        {min.x, min.y, max.z}, {max.x, min.y, max.z}, {min.x, max.y, max.z}, {max.x, max.y, max.z}
    }};

    float screenMinX = GetSize().x, screenMinY = GetSize().y;
    float screenMaxX = 0.0f, screenMaxY = 0.0f;
    bool anyVisible = false;

    for (const auto& corner : corners) {
        bool visible = false;
        dr4::Vec2f screenPos = ProjectToScreen(corner, visible);
        if (!visible) continue;
        anyVisible = true;
        screenMinX = std::min(screenMinX, screenPos.x);
        screenMinY = std::min(screenMinY, screenPos.y);
        screenMaxX = std::max(screenMaxX, screenPos.x);
        screenMaxY = std::max(screenMaxY, screenPos.y);
    }

    if (!anyVisible) {
        return;
    }

    auto* rect = GetUI()->GetWindow()->CreateRectangle();
    rect->SetPos(dr4::Vec2f(screenMinX, screenMinY));
    rect->SetSize(dr4::Vec2f(screenMaxX - screenMinX, screenMaxY - screenMinY));
    rect->SetFillColor(dr4::Color(0, 0, 0, 0));
    rect->SetBorderColor(dr4::Color(255, 255, 0));
    rect->SetBorderThickness(2.0f);
    texture.Draw(*rect);
}

hui::EventResult RayTracerWindow::OnMouseDown(hui::MouseButtonEvent& evt) {
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (debugInput) {
        std::cout << "[ui] RayTracerWindow MouseDown pos=(" << evt.pos.x << "," << evt.pos.y << ")\n";
    }
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (evt.pos.y <= titleBarHeight) {
            if (evt.pos.x > GetSize().x - 30) {
                isCollapsed = !isCollapsed;
                
                if (isCollapsed) {
                    expandedSize = GetSize();
                    hasExpandedSize = true;
                    SetSize(GetSize().x, titleBarHeight);
                } else if (hasExpandedSize) {
                    SetSize(expandedSize);
                }
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
            dragging = true;
            dragOffset = evt.pos;
            GetUI()->ReportFocus(this);
            return hui::EventResult::HANDLED;
        }
    }

    
    if (isCollapsed) {
        return hui::EventResult::UNHANDLED;
    }
    if (evt.button == dr4::MouseButtonType::LEFT && scene && camera && !isCollapsed) {
        float screenX = evt.pos.x;
        float screenY = evt.pos.y - titleBarHeight;
        if (screenY < 0) return hui::EventResult::HANDLED;
        
        float viewportH = std::max(1.0f, GetSize().y - titleBarHeight);
        if (screenX >= 0 && screenX < GetSize().x && screenY >= 0 && screenY < viewportH) {
            raytracer::Ray ray = camera->GetRay(screenX, screenY, GetSize().x, viewportH);
            raytracer::HitResult closestHit;
            closestHit.t = 1e10f;
            
            for (auto& obj : scene->objects) {
                raytracer::HitResult hit = obj->Intersect(ray);
                if (hit.hit && hit.t < closestHit.t && hit.t > 0.001f) {
                    closestHit = hit;
                }
            }
            
            if (closestHit.hit && closestHit.object) {
                SetSelectedObject(closestHit.object);
                if (onObjectSelected) {
                    onObjectSelected(const_cast<raytracer::Object*>(closestHit.object));
                }
                if (debugInput) {
                    std::cout << "[ui] RayTracerWindow select object via raycast\n";
                }
                return hui::EventResult::HANDLED;
            }
        }
    }
    
    return Widget::OnMouseDown(evt);
}

hui::EventResult RayTracerWindow::OnMouseMove(hui::MouseMoveEvent& evt) {
    GetUI()->ReportHover(this);
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (dragging) {
        dr4::Vec2f delta = evt.pos - dragOffset;
        SetPos(GetPos() + delta);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (isCollapsed) {
        return hui::EventResult::UNHANDLED;
    }
    if (debugInput) {
        std::cout << "[ui] RayTracerWindow MouseMove pos=(" << evt.pos.x << "," << evt.pos.y << ")\n";
    }
    return Widget::OnMouseMove(evt);
}

hui::EventResult RayTracerWindow::OnKeyDown(hui::KeyEvent& evt) {
    if (evt.key == dr4::KEYCODE_V && (evt.mods & dr4::KEYMOD_CTRL)) {
        if (onPasteRequest) {
            onPasteRequest();
        }
        return hui::EventResult::HANDLED;
    }
    return Widget::OnKeyDown(evt);
}

hui::EventResult RayTracerWindow::OnIdle(hui::IdleEvent& evt) {
    (void)evt;
    return Widget::OnIdle(evt);
}

hui::EventResult RayTracerWindow::OnMouseUp(hui::MouseButtonEvent& evt) {
    if (dragging) {
        dragging = false;
        return hui::EventResult::HANDLED;
    }
    return Widget::OnMouseUp(evt);
}

dr4::Vec2f RayTracerWindow::ProjectToScreen(const raytracer::Vec3& point, bool& visible) const {
    visible = false;
    raytracer::Vec3 toPoint = point - camera->position;
    raytracer::Vec3 forward = camera->GetForward();
    raytracer::Vec3 right = camera->GetRight();
    raytracer::Vec3 up = camera->GetUp();

    float localZ = toPoint.Dot(forward);
    if (localZ <= 0.001f) {
        return {};
    }

    float localX = toPoint.Dot(right);
    float localY = toPoint.Dot(up);

    float tanHalfFov = std::tan(camera->fov * 3.14159f / 180.0f * 0.5f);
    float ndcX = (localX / (localZ * tanHalfFov * camera->aspectRatio));
    float ndcY = (localY / (localZ * tanHalfFov));

    float viewportH = std::max(1.0f, GetSize().y - titleBarHeight);
    float screenX = (ndcX + 1.0f) * 0.5f * GetSize().x;
    float screenY = (1.0f - (ndcY + 1.0f) * 0.5f) * viewportH + titleBarHeight;

    visible = screenX >= 0 && screenX <= GetSize().x && screenY >= titleBarHeight && screenY <= titleBarHeight + viewportH;
    return dr4::Vec2f(screenX, screenY);
}

} // namespace ui