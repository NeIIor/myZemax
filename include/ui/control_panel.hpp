#ifndef UI_CONTROL_PANEL_HPP
#define UI_CONTROL_PANEL_HPP

#include "hui/container.hpp"
#include "raytracer/camera.hpp"
#include <functional>

namespace hui {
    class IdleEvent;
    class KeyEvent;
    class MouseButtonEvent;
}

namespace ui {

class ControlPanel : public hui::Container {
public:
    ControlPanel(hui::UI* ui, raytracer::Camera* camera);
    
    void UpdateFromCamera();
    
protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnIdle(hui::IdleEvent& evt) override;
    hui::EventResult OnKeyDown(hui::KeyEvent& evt) override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnKeyUp(hui::KeyEvent& evt) override;

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
    bool isCollapsed = false;
    double lastTime = 0.0f;
    
    std::string posXText, posYText, posZText;
    std::string lookAtXText, lookAtYText, lookAtZText;
    std::string fovText;
    
    int activeField = -1;
    
    void ParseAndApplyChanges();
};

} // namespace ui

#endif // UI_CONTROL_PANEL_HPP