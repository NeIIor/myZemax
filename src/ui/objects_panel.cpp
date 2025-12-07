#include "ui/objects_panel.hpp"
#include "raytracer/object.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"  // Добавляю для GetUI()->GetWindow()

namespace ui {

ObjectsPanel::ObjectsPanel(hui::UI* ui, raytracer::Scene* scene_)
    : Container(ui), scene(scene_), isExpanded(false), scrollOffset(0.0f) {
    SetSize(200, 300);
}

hui::EventResult ObjectsPanel::PropagateToChildren(hui::Event& event) {
    return hui::EventResult::UNHANDLED;
}

void ObjectsPanel::RefreshList() {
    ForceRedraw();
}

void ObjectsPanel::SetSelectedObject(raytracer::Object* obj) {
    selectedObject = obj;
    ForceRedraw();
}

void ObjectsPanel::Redraw() const {
    dr4::Texture& texture = GetTexture();
    texture.Clear(dr4::Color(50, 50, 50));
    
    auto* text = GetUI()->GetWindow()->CreateText();
    text->SetText("Objects");
    text->SetPos(dr4::Vec2f(10, 5));
    text->SetFontSize(14);
    text->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*text);
    
    auto* arrowText = GetUI()->GetWindow()->CreateText();
    arrowText->SetText(isExpanded ? "▼" : "▶");
    arrowText->SetPos(dr4::Vec2f(GetSize().x - 20, 5));
    arrowText->SetFontSize(14);
    arrowText->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*arrowText);
    
    if (isExpanded && scene) {
        DrawObjectList(texture);
    }
}

void ObjectsPanel::DrawObjectList(dr4::Texture& texture) const {
    if (!scene) return;
    
    float y = 25.0f;
    float itemHeight = 25.0f;
    float startY = 25.0f - scrollOffset;
    
    for (size_t i = 0; i < scene->objects.size(); ++i) {
        auto& obj = scene->objects[i];
        float itemY = startY + i * itemHeight;
        
        if (itemY < 0 || itemY > GetSize().y) continue;
        
        if (obj.get() == selectedObject) {
            auto* rect = GetUI()->GetWindow()->CreateRectangle();
            rect->SetPos(dr4::Vec2f(0, itemY));
            rect->SetSize(dr4::Vec2f(GetSize().x, itemHeight));
            rect->SetFillColor(dr4::Color(100, 150, 255, 100));
            texture.Draw(*rect);
        }
        
        auto* text = GetUI()->GetWindow()->CreateText();
        text->SetText(obj->name);
        text->SetPos(dr4::Vec2f(10, itemY + 5));
        text->SetFontSize(12);
        text->SetColor(dr4::Color(255, 255, 255));
        texture.Draw(*text);
    }
}

raytracer::Object* ObjectsPanel::GetObjectAtPosition(const dr4::Vec2f& pos) const {
    if (!scene || !isExpanded) return nullptr;
    
    float itemHeight = 25.0f;
    float startY = 25.0f - scrollOffset;
    
    if (pos.y < startY) return nullptr;
    
    int index = static_cast<int>((pos.y - startY) / itemHeight);
    if (index >= 0 && index < static_cast<int>(scene->objects.size())) {
        return scene->objects[index].get();
    }
    
    return nullptr;
}

hui::EventResult ObjectsPanel::OnMouseDown(hui::MouseButtonEvent& evt) {
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (evt.pos.x > GetSize().x - 30 && evt.pos.y < 25) {
            isExpanded = !isExpanded;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        
        if (isExpanded) {
            raytracer::Object* obj = GetObjectAtPosition(evt.pos);
            if (obj) {
                SetSelectedObject(obj);
                if (onObjectSelected) {
                    onObjectSelected(obj);
                }
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }
    }
    
    return Container::OnMouseDown(evt);
}

} // namespace ui