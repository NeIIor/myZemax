#include "ui/objects_panel.hpp"
#include "raytracer/object.hpp"
#include "raytracer/objects.hpp"
#include "dr4/math/rect.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include <cmath>
#include <memory>
#include <cstdlib>
#include <iostream>

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
    const dr4::Color bgPanel(58, 58, 58);
    const dr4::Color bgHeader(76, 76, 76);
    const dr4::Color borderHeader(96, 96, 96);
    const dr4::Color textMain(235, 235, 235);
    const dr4::Color textSub(210, 210, 210);
    const dr4::Color btnBase(68, 68, 68);
    const dr4::Color btnHover(88, 98, 110);
    const dr4::Color btnPressed(58, 58, 60);


    texture.Clear(isExpanded ? bgPanel : dr4::Color(0, 0, 0, 0));
    
    auto* bar = GetUI()->GetWindow()->CreateRectangle();
    bar->SetPos(dr4::Vec2f(0, 0));
    bar->SetSize(dr4::Vec2f(GetSize().x, titleBarHeight));
    bar->SetFillColor(bgHeader);
    bar->SetBorderColor(borderHeader);
    bar->SetBorderThickness(1.0f);
    texture.Draw(*bar);
    
    auto* text = GetUI()->GetWindow()->CreateText();
    text->SetText("Objects");
    text->SetPos(dr4::Vec2f(12, 6));
    text->SetFontSize(14);
    text->SetColor(textMain);
    texture.Draw(*text);
    
    auto* arrowText = GetUI()->GetWindow()->CreateText();
    arrowText->SetText(isExpanded ? "v" : ">");
    arrowText->SetPos(dr4::Vec2f(GetSize().x - 24, 6));
    arrowText->SetFontSize(14);
    arrowText->SetColor(textMain);
    texture.Draw(*arrowText);
    
    if (isExpanded && scene) {
        float addAreaHeight = 44.0f;
        float baseY = titleBarHeight;

        auto* addLabel = GetUI()->GetWindow()->CreateText();
        addLabel->SetText("Add object:");
        addLabel->SetPos(dr4::Vec2f(12, baseY + 6));
        addLabel->SetFontSize(13);
        addLabel->SetColor(textSub);
        texture.Draw(*addLabel);

        dr4::Rect2f addBtn(dr4::Vec2f(12.0f, baseY + 22.0f), dr4::Vec2f(GetSize().x - 24.0f, 20.0f));
        auto* rect = GetUI()->GetWindow()->CreateRectangle();
        rect->SetPos(addBtn.pos);
        rect->SetSize(addBtn.size);
        dr4::Color fill = btnBase;
        if (pressedAdd) fill = btnPressed;
        else if (hoverAdd) fill = btnHover;
        rect->SetFillColor(fill);
        rect->SetBorderColor(borderHeader);
        rect->SetBorderThickness(1.0f);
        texture.Draw(*rect);

        auto* label = GetUI()->GetWindow()->CreateText();
        label->SetText("+ Add...");
        label->SetPos(dr4::Vec2f(addBtn.pos.x + 8, addBtn.pos.y + (pressedAdd ? 5.0f : 4.0f)));
        label->SetFontSize(12);
        label->SetColor(textMain);
        texture.Draw(*label);

        DrawObjectList(texture);
    }
}

void ObjectsPanel::DrawObjectList(dr4::Texture& texture) const {
    if (!scene) return;
    
    const float headerHeight = titleBarHeight;
    const float addAreaHeight = 44.0f;
    const float listPaddingTop = 10.0f;
    const float listStart = headerHeight + addAreaHeight + listPaddingTop;
    float itemHeight = 26.0f;
    float startY = listStart - scrollOffset;
    
    for (size_t i = 0; i < scene->objects.size(); ++i) {
        auto& obj = scene->objects[i];
        float itemY = startY + i * itemHeight;
        
        
        
        if (itemY < listStart || itemY > GetSize().y) continue;
        
        if (obj.get() == selectedObject) {
            auto* rect = GetUI()->GetWindow()->CreateRectangle();
            rect->SetPos(dr4::Vec2f(0, itemY));
            rect->SetSize(dr4::Vec2f(GetSize().x, itemHeight));
            rect->SetFillColor(dr4::Color(120, 170, 230, 110));
            texture.Draw(*rect);
        }
        
        auto* text = GetUI()->GetWindow()->CreateText();
        text->SetText(GetTypeLabel(obj.get()) + " " + obj->name);
        text->SetPos(dr4::Vec2f(10, itemY + 4));
        text->SetFontSize(12);
        text->SetColor(obj->isLightSource ? dr4::Color(255, 230, 180) : dr4::Color(255, 255, 255));
        texture.Draw(*text);
    }
}

raytracer::Object* ObjectsPanel::GetObjectAtPosition(const dr4::Vec2f& pos) const {
    if (!scene || !isExpanded) return nullptr;
    
    const float headerHeight = titleBarHeight;
    const float addAreaHeight = 44.0f;
    const float listPaddingTop = 10.0f;
    const float listStart = headerHeight + addAreaHeight + listPaddingTop;
    const float itemHeight = 26.0f;
    if (pos.y < listStart || pos.y > GetSize().y) return nullptr;

    int index = static_cast<int>(((pos.y - listStart) + scrollOffset) / itemHeight);
    if (index >= 0 && index < static_cast<int>(scene->objects.size())) {
        return scene->objects[index].get();
    }
    
    return nullptr;
}

hui::EventResult ObjectsPanel::OnMouseDown(hui::MouseButtonEvent& evt) {
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (debugInput) {
        std::cout << "[ui] ObjectsPanel MouseDown pos=(" << evt.pos.x << "," << evt.pos.y << ")\n";
    }
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (evt.pos.y <= titleBarHeight) {
            if (evt.pos.x > GetSize().x - 30) {
                isExpanded = !isExpanded;
                
                if (!isExpanded) {
                    expandedSize = GetSize();
                    hasExpandedSize = true;
                    SetSize(GetSize().x, titleBarHeight);
                } else if (hasExpandedSize) {
                    SetSize(expandedSize);
                }
                ForceRedraw();
                GetUI()->ReportFocus(this);
                return hui::EventResult::HANDLED;
            }
            dragging = true;
            dragOffset = evt.pos;
            GetUI()->ReportFocus(this);
            return hui::EventResult::HANDLED;
        }

        if (!isExpanded) {
            
            return hui::EventResult::UNHANDLED;
        }

        if (isExpanded) {
            dr4::Rect2f addBtn(dr4::Vec2f(12.0f, titleBarHeight + 22.0f), dr4::Vec2f(GetSize().x - 24.0f, 20.0f));
            if (addBtn.Contains(evt.pos)) {
                pressedAdd = true;
                ForceRedraw();
                GetUI()->ReportFocus(this);
                return hui::EventResult::HANDLED;
            }

            raytracer::Object* obj = GetObjectAtPosition(evt.pos);
            if (obj) {
                SetSelectedObject(obj);
                if (onObjectSelected) {
                    onObjectSelected(obj);
                }
                GetUI()->ReportFocus(this);
                if (onSceneChanged) onSceneChanged();
                ForceRedraw();
                if (debugInput) {
                    std::cout << "[ui] ObjectsPanel select object\n";
                }
                return hui::EventResult::HANDLED;
            }
        }
    }
    
    return Container::OnMouseDown(evt);
}

hui::EventResult ObjectsPanel::OnMouseMove(hui::MouseMoveEvent& evt) {
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (dragging) {
        dr4::Vec2f delta = evt.pos - dragOffset;
        SetPos(GetPos() + delta);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (!isExpanded) return hui::EventResult::UNHANDLED;

    dr4::Rect2f addBtn(dr4::Vec2f(12.0f, titleBarHeight + 22.0f), dr4::Vec2f(GetSize().x - 24.0f, 20.0f));
    bool newHover = addBtn.Contains(evt.pos);
    if (newHover != hoverAdd) {
        hoverAdd = newHover;
        ForceRedraw();
    }
    return Container::OnMouseMove(evt);
}

hui::EventResult ObjectsPanel::OnMouseWheel(hui::MouseWheelEvent& evt) {
    if (!scene || !isExpanded) return hui::EventResult::UNHANDLED;

    const float headerHeight = titleBarHeight;
    const float addAreaHeight = 44.0f;
    const float listPaddingTop = 10.0f;
    const float listStart = headerHeight + addAreaHeight + listPaddingTop;
    const float itemHeight = 26.0f;
    float visibleHeight = GetSize().y - listStart;
    float totalHeight = static_cast<float>(scene->objects.size()) * itemHeight;
    float maxScroll = std::max(0.0f, totalHeight - visibleHeight);

    scrollOffset -= evt.delta.y * 20.0f;
    scrollOffset = std::max(0.0f, std::min(scrollOffset, maxScroll));
    ForceRedraw();
    return hui::EventResult::HANDLED;
}

hui::EventResult ObjectsPanel::OnMouseUp(hui::MouseButtonEvent& evt) {
    if (dragging) {
        dragging = false;
        return hui::EventResult::HANDLED;
    }
    if (!isExpanded) {
        return hui::EventResult::UNHANDLED;
    }
    if (evt.button == dr4::MouseButtonType::LEFT && pressedAdd) {
        dr4::Rect2f addBtn(dr4::Vec2f(12.0f, titleBarHeight + 22.0f), dr4::Vec2f(GetSize().x - 24.0f, 20.0f));
        bool fire = addBtn.Contains(evt.pos);
        pressedAdd = false;
        ForceRedraw();
        if (fire) {
            if (onAddRequested) onAddRequested();
            return hui::EventResult::HANDLED;
        }
        return hui::EventResult::HANDLED;
    }
    return Container::OnMouseUp(evt);
}

std::string ObjectsPanel::MakeUniqueName(const std::string& base) const {
    int counter = 1;
    std::string candidate = base;
    while (scene && scene->FindObjectByName(candidate)) {
        candidate = base + " " + std::to_string(counter++);
    }
    return candidate;
}

std::string ObjectsPanel::GetTypeLabel(const raytracer::Object* obj) const {
    if (dynamic_cast<const raytracer::Sphere*>(obj)) return "[Sphere]";
    if (dynamic_cast<const raytracer::Plane*>(obj)) return "[Plane]";
    if (dynamic_cast<const raytracer::RectPlane*>(obj)) return "[BoundedPlane]";
    if (dynamic_cast<const raytracer::Disk*>(obj)) return "[Disk]";
    if (dynamic_cast<const raytracer::Prism*>(obj)) return "[Prism]";
    if (dynamic_cast<const raytracer::Pyramid*>(obj)) return "[Pyramid]";
    return "[Obj]";
}

} // namespace ui