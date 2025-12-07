#ifndef UI_PROPERTIES_WINDOW_HPP
#define UI_PROPERTIES_WINDOW_HPP

#include "hui/container.hpp"
#include "raytracer/object.hpp"
#include <functional>

namespace ui {

class PropertiesWindow : public hui::Container {
public:
    PropertiesWindow(hui::UI* ui);
    
    void SetObject(raytracer::Object* obj);
    void SetCollapsed(bool collapsed) { isCollapsed = collapsed; ForceRedraw(); }
    bool IsCollapsed() const { return isCollapsed; }
    raytracer::Object* GetObject() const { return currentObject; }
    void UpdateFromObject();
    
    void CopyObject();
    void PasteObject(raytracer::Scene* scene);
    bool HasCopiedObject() const { return copiedObject != nullptr; }
    void SetOnPasteRequest(std::function<void()> callback) { onPasteRequest = callback; }

protected:
    void Redraw() const override;
    EventResult OnText(TextEvent& evt) override;
    EventResult OnKeyDown(KeyEvent& evt) override;
    EventResult OnMouseDown(MouseButtonEvent& evt) override;

private:
    raytracer::Object* currentObject = nullptr;
    bool isCollapsed = false;
    
    std::string nameText;
    std::string posXText, posYText, posZText;
    std::string colorRText, colorGText, colorBText;
    std::string refractiveIndexText;
    std::string reflectivityText;
    
    int activeField = -1;
    raytracer::Object* copiedObject = nullptr;
    std::function<void()> onPasteRequest;
    
    void ParseAndApplyChanges();
    raytracer::Object* CloneObject(raytracer::Object* obj) const;
};

} // namespace ui

#endif // UI_PROPERTIES_WINDOW_HPP



