#ifndef UI_CONTROL_PANEL_HPP
#define UI_CONTROL_PANEL_HPP

#include "hui/container.hpp"
#include "raytracer/camera.hpp"

namespace ui {

class ControlPanel : public hui::Container {
public:
    ControlPanel(hui::UI* ui, raytracer::Camera* camera);
    void SetupButtons();
    
    void SetCollapsed(bool collapsed) { isCollapsed = collapsed; ForceRedraw(); }
    bool IsCollapsed() const { return isCollapsed; }

protected:
    void Redraw() const override;
    EventResult OnIdle(IdleEvent& evt) override;

private:
    raytracer::Camera* camera;
    
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
    bool rotateLeft = false;
    bool rotateRight = false;
    bool rotateUp = false;
    bool rotateDown = false;
    
    float lastTime = 0.0f;
    bool isCollapsed = false;
    
    EventResult OnKeyDown(KeyEvent& evt) override;
    EventResult OnMouseDown(MouseButtonEvent& evt) override;
    EventResult OnKeyUp(KeyEvent& evt) override;
};

} // namespace ui

#endif // UI_CONTROL_PANEL_HPP

