#ifndef UI_RAYTRACER_WINDOW_HPP
#define UI_RAYTRACER_WINDOW_HPP

#include "hui/widget.hpp"
#include "raytracer/raytracer.hpp"
#include "raytracer/scene.hpp"
#include "raytracer/camera.hpp"
#include "raytracer/object.hpp"
#include <functional>

namespace hui {
    class MouseButtonEvent;
    class KeyEvent;
    class IdleEvent;
}

namespace ui {

class RayTracerWindow : public hui::Widget {
public:
    RayTracerWindow(hui::UI* ui, raytracer::Scene* scene, raytracer::Camera* camera);
    
    void SetRayTracer(raytracer::RayTracer* rt) { raytracer = rt; needsRender = true; renderDelayFrames = 0; }
    void SetSelectedObject(raytracer::Object* obj) { selectedObject = obj; ForceRedraw(); }
    void SetSelectedObject(const raytracer::Object* obj); 
    raytracer::Object* GetSelectedObject() const { return selectedObject; }
    void MarkDirty();
    void SetOnPasteRequest(std::function<void()> callback) { onPasteRequest = callback; }
    void SetOnObjectSelected(std::function<void(raytracer::Object*)> callback) { onObjectSelected = std::move(callback); }

protected:
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnKeyDown(hui::KeyEvent& evt) override;
    hui::EventResult OnIdle(hui::IdleEvent& evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent& evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent& evt) override;

private:
    raytracer::Scene* scene;
    raytracer::Camera* camera;
    raytracer::RayTracer* raytracer;
    raytracer::Object* selectedObject = nullptr;
    bool isCollapsed = false;
    float titleBarHeight = 26.0f;
    bool dragging = false;
    dr4::Vec2f dragOffset;
    dr4::Vec2f expandedSize{0.0f, 0.0f};
    bool hasExpandedSize = false;
    
    mutable dr4::Image* renderImage = nullptr; 
    mutable bool needsRender = true; 
    mutable int renderDelayFrames = 0;
    std::function<void()> onPasteRequest;
    std::function<void(raytracer::Object*)> onObjectSelected;
    
    void DrawSelectionBox(dr4::Texture& texture, raytracer::Object* obj) const;
    dr4::Vec2f ProjectToScreen(const raytracer::Vec3& point, bool& visible) const;
};

} // namespace ui

#endif // UI_RAYTRACER_WINDOW_HPP