#ifndef UI_PROPERTIES_WINDOW_HPP
#define UI_PROPERTIES_WINDOW_HPP

#include "hui/container.hpp"
#include "raytracer/object.hpp"
#include <memory>
#include <array>
#include <functional>

namespace hui {
    class TextEvent;
    class KeyEvent;
    class MouseButtonEvent;
}

namespace raytracer {
    class Scene;  
}

namespace ui {

class PropertiesWindow : public hui::Container {
public:
    PropertiesWindow(hui::UI* ui);
    
    void SetObject(raytracer::Object* obj);
    void StartDraft(raytracer::Scene* scene, std::unique_ptr<raytracer::Object> obj);
    void SetCollapsed(bool collapsed);
    bool IsCollapsed() const { return isCollapsed; }
    raytracer::Object* GetObject() const { return currentObject; }
    void UpdateFromObject();
    
    void CopyObject();
    void PasteObject(raytracer::Scene* scene);
    bool HasCopiedObject() const { return copiedObject != nullptr; }
    void SetOnPasteRequest(std::function<void()> callback) { onPasteRequest = callback; }
    void SetOnObjectChanged(std::function<void()> callback) { onObjectChanged = std::move(callback); }
    void SetOnObjectCommitted(std::function<void(raytracer::Object*)> callback) { onObjectCommitted = std::move(callback); }

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnText(hui::TextEvent& evt) override;
    hui::EventResult OnIdle(hui::IdleEvent& evt) override;
    hui::EventResult OnKeyDown(hui::KeyEvent& evt) override;
    hui::EventResult OnKeyUp(hui::KeyEvent& evt) override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent& evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent& evt) override;

private:
    enum class ButtonId : uint8_t { None = 0, Copy, Paste, Enter };
    enum class ObjKind : uint8_t { Unknown = 0, Sphere, Plane, RectPlane, Disk, Prism, Pyramid, Light };

    raytracer::Object* currentObject = nullptr;
    ObjKind currentKind = ObjKind::Unknown;
    bool isCollapsed = false;
    float titleBarHeight = 26.0f;
    bool dragging = false;
    dr4::Vec2f dragOffset;
    dr4::Vec2f expandedSize{0.0f, 0.0f};
    bool hasExpandedSize = false;
    
    std::string nameText;
    std::string posXText, posYText, posZText;
    std::string colorRText, colorGText, colorBText;
    std::string refractiveIndexText;
    std::string reflectivityText;

    std::string sphereRadiusText;
    std::string planeNxText, planeNyText, planeNzText;
    std::string rectPlaneNxText, rectPlaneNyText, rectPlaneNzText;
    std::string rectPlaneWidthText, rectPlaneHeightText;
    std::string diskRadiusText, diskNxText, diskNyText, diskNzText;
    std::string prismSizeXText, prismSizeYText, prismSizeZText;
    std::string pyramidBaseText, pyramidHeightText;

    std::unique_ptr<raytracer::Object> draftObject;
    raytracer::Scene* draftScene = nullptr;
    
    int activeField = -1;
    size_t caretPos = 0;
    size_t selStart = 0;
    size_t selEnd = 0;
    bool selectingText = false;
    bool selectionDragging = false;
    size_t selectionAnchor = 0;
    dr4::Vec2f selectionMouseDownPos;
    bool caretVisible = true;
    double caretBlinkAccum = 0.0;
    std::array<bool, 256> keyHeld{};
    ButtonId hoveredButton = ButtonId::None;
    ButtonId pressedButton = ButtonId::None;
    raytracer::Object* copiedObject = nullptr;
    std::function<void()> onPasteRequest;
    std::function<void()> onObjectChanged;
    std::function<void(raytracer::Object*)> onObjectCommitted;
    
    void ParseAndApplyChanges();
    raytracer::Object* CloneObject(raytracer::Object* obj) const;
    std::string* GetFieldByIndex(int idx);
    void ApplyTextInput(const char* text);
    bool IsNumericField(int idx) const;
    int FieldCount() const;
    float FieldValueOffset(int idx) const;
};

} // namespace ui

#endif // UI_PROPERTIES_WINDOW_HPP