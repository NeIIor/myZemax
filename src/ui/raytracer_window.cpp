#include "ui/raytracer_window.hpp"
#include "raytracer/objects.hpp"
#include "dr4/keycodes.hpp"
#include <cmath>
#include <algorithm>

namespace ui {

RayTracerWindow::RayTracerWindow(hui::UI* ui, raytracer::Scene* scene_, raytracer::Camera* camera_)
    : Widget(ui), scene(scene_), camera(camera_), raytracer(nullptr) {
    SetSize(800, 600);
}

void RayTracerWindow::Redraw() const {
    dr4::Texture& texture = GetTexture();
    texture.Clear(dr4::Color(20, 20, 30));
    
    if (!raytracer || !scene || !camera) {
        return;
    }
    
    if (!renderImage) {
        renderImage = GetWindow()->CreateImage();
        renderImage->SetSize(GetSize());
    }
    
    if (needsRender || !renderImage) {
        raytracer->Render(renderImage);
        needsRender = false;
    }
    
    if (renderImage) {
        renderImage->SetPos(GetPos());
        texture.Draw(*renderImage);
    }
    
    if (selectedObject) {
        DrawSelectionBox(texture, selectedObject);
    }
}

void RayTracerWindow::DrawSelectionBox(dr4::Texture& texture, raytracer::Object* obj) const {
    raytracer::Vec3 min, max;
    obj->GetBoundingBox(min, max);
    
    dr4::Vec2f screenMin, screenMax;
    screenMin = dr4::Vec2f(min.x * 50 + GetSize().x * 0.5f, min.y * 50 + GetSize().y * 0.5f);
    screenMax = dr4::Vec2f(max.x * 50 + GetSize().x * 0.5f, max.y * 50 + GetSize().y * 0.5f);
    
    auto* rect = GetWindow()->CreateRectangle();
    rect->SetPos(screenMin);
    rect->SetSize(screenMax - screenMin);
    rect->SetFillColor(dr4::Color(0, 0, 0, 0));
    rect->SetBorderColor(dr4::Color(255, 255, 0));
    rect->SetBorderThickness(2.0f);
    texture.Draw(*rect);
}

EventResult RayTracerWindow::OnMouseDown(MouseButtonEvent& evt) {
    if (evt.button == dr4::MouseButtonType::LEFT && scene && camera) {
        float screenX = evt.pos.x - GetPos().x;
        float screenY = evt.pos.y - GetPos().y;
        
        if (screenX >= 0 && screenX < GetSize().x && screenY >= 0 && screenY < GetSize().y) {
            raytracer::Ray ray = camera->GetRay(screenX, screenY, GetSize().x, GetSize().y);
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
                return EventResult::HANDLED;
            }
        }
    }
    
    return Widget::OnMouseDown(evt);
}

EventResult RayTracerWindow::OnKeyDown(KeyEvent& evt) {
    if (evt.key == dr4::KEYCODE_V && (evt.mods & dr4::KEYMOD_CTRL)) {
        if (onPasteRequest) {
            onPasteRequest();
        }
        return EventResult::HANDLED;
    }
    return Widget::OnKeyDown(evt);
}

EventResult RayTracerWindow::OnIdle(IdleEvent& evt) {
    if (needsRender) {
        ForceRedraw();
    }
    return Widget::OnIdle(evt);
}

} // namespace ui

