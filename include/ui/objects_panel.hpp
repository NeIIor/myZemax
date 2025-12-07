#ifndef UI_OBJECTS_PANEL_HPP
#define UI_OBJECTS_PANEL_HPP

#include "hui/container.hpp"
#include "raytracer/scene.hpp"
#include <functional>

namespace hui {
    class MouseButtonEvent;
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

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;

private:
    raytracer::Scene* scene;
    raytracer::Object* selectedObject = nullptr;
    bool isExpanded = false;
    float scrollOffset = 0.0f;
    
    std::function<void(raytracer::Object*)> onObjectSelected;
    
    void DrawObjectList(dr4::Texture& texture) const;
    raytracer::Object* GetObjectAtPosition(const dr4::Vec2f& pos) const;
};

} // namespace ui

#endif // UI_OBJECTS_PANEL_HPP