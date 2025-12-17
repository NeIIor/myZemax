#ifndef UI_OBJECTS_PANEL_HPP
#define UI_OBJECTS_PANEL_HPP

#include "hui/container.hpp"
#include "raytracer/scene.hpp"
#include <functional>
#include <vector>
#include <string>

namespace hui {
    class MouseButtonEvent;
    class MouseWheelEvent;
}

namespace ui {

class ObjectsPanel : public hui::Container {
public:
    ObjectsPanel(hui::UI* ui, raytracer::Scene* scene);
    
    void SetOnObjectSelected(std::function<void(raytracer::Object*)> callback) {
        onObjectSelected = callback;
    }
    
    void RefreshList();
    void SetSelectedObject(raytracer::Object* obj);
    void SetOnSceneChanged(std::function<void()> callback) { onSceneChanged = std::move(callback); }
    void SetOnAddRequested(std::function<void()> callback) { onAddRequested = std::move(callback); }

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent& evt) override;
    hui::EventResult OnMouseWheel(hui::MouseWheelEvent& evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent& evt) override;

private:
    raytracer::Scene* scene;
    raytracer::Object* selectedObject = nullptr;
    bool isExpanded = false;
    float scrollOffset = 0.0f;
    bool hoverAdd = false;
    bool pressedAdd = false;
    float titleBarHeight = 26.0f;
    bool dragging = false;
    dr4::Vec2f dragOffset;
    dr4::Vec2f expandedSize{0.0f, 0.0f};
    bool hasExpandedSize = false;
    
    std::function<void(raytracer::Object*)> onObjectSelected;
    std::function<void()> onSceneChanged;
    std::function<void()> onAddRequested;
    
    void DrawObjectList(dr4::Texture& texture) const;
    raytracer::Object* GetObjectAtPosition(const dr4::Vec2f& pos) const;
    std::string MakeUniqueName(const std::string& base) const;
    std::string GetTypeLabel(const raytracer::Object* obj) const;
};

} // namespace ui

#endif // UI_OBJECTS_PANEL_HPP