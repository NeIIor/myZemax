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
    
    void SetRayTracer(raytracer::RayTracer* rt) { raytracer = rt; needsRender = true; }
    void SetSelectedObject(raytracer::Object* obj) { selectedObject = obj; ForceRedraw(); }
    void SetSelectedObject(const raytracer::Object* obj); // Добавляю перегрузку для const
    raytracer::Object* GetSelectedObject() const { return selectedObject; }
    void MarkDirty() { needsRender = true; }
    void SetOnPasteRequest(std::function<void()> callback) { onPasteRequest = callback; }

protected:
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnKeyDown(hui::KeyEvent& evt) override;
    hui::EventResult OnIdle(hui::IdleEvent& evt) override;

private:
    raytracer::Scene* scene;
    raytracer::Camera* camera;
    raytracer::RayTracer* raytracer;
    raytracer::Object* selectedObject = nullptr;
    
    mutable dr4::Image* renderImage = nullptr; // Добавляю mutable
    mutable bool needsRender = true; // Добавляю mutable
    std::function<void()> onPasteRequest;
    
    void DrawSelectionBox(dr4::Texture& texture, raytracer::Object* obj) const;
};

} // namespace ui

#endif // UI_RAYTRACER_WINDOW_HPP