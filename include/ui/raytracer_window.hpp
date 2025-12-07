#ifndef UI_RAYTRACER_WINDOW_HPP
#define UI_RAYTRACER_WINDOW_HPP

#include "hui/widget.hpp"
#include "raytracer/raytracer.hpp"
#include "raytracer/scene.hpp"
#include "raytracer/camera.hpp"
#include "raytracer/object.hpp"
#include <functional>

namespace ui {

class RayTracerWindow : public hui::Widget {
public:
    RayTracerWindow(hui::UI* ui, raytracer::Scene* scene, raytracer::Camera* camera);
    
    void SetRayTracer(raytracer::RayTracer* rt) { raytracer = rt; needsRender = true; }
    void SetSelectedObject(raytracer::Object* obj) { selectedObject = obj; ForceRedraw(); }
    raytracer::Object* GetSelectedObject() const { return selectedObject; }
    void MarkDirty() { needsRender = true; }
    void SetOnPasteRequest(std::function<void()> callback) { onPasteRequest = callback; }

protected:
    void Redraw() const override;
    EventResult OnMouseDown(MouseButtonEvent& evt) override;
    EventResult OnKeyDown(KeyEvent& evt) override;
    EventResult OnIdle(IdleEvent& evt) override;

private:
    raytracer::Scene* scene;
    raytracer::Camera* camera;
    raytracer::RayTracer* raytracer;
    raytracer::Object* selectedObject = nullptr;
    
    dr4::Image* renderImage = nullptr;
    bool needsRender = true;
    std::function<void()> onPasteRequest;
    
    void DrawSelectionBox(dr4::Texture& texture, raytracer::Object* obj) const;
};

} // namespace ui

#endif // UI_RAYTRACER_WINDOW_HPP

