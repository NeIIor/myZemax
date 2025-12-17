#ifndef UI_CONTROL_PANEL_HPP
#define UI_CONTROL_PANEL_HPP

#include "hui/container.hpp"
#include "raytracer/camera.hpp"
#include <functional>
#include <vector>
#include <cstdint>

namespace hui {
    class IdleEvent;
    class KeyEvent;
    class MouseButtonEvent;
}

namespace ui {

enum class ControlPanelIcon : uint8_t {
    Forward,
    Back,
    Left,
    Right,
    Up,
    Down,
    YawLeft,
    YawRight,
    PitchUp,
    PitchDown,
    ResetCam,
};

class ControlPanel : public hui::Container {
public:
    ControlPanel(hui::UI* ui, raytracer::Camera* camera);
    
    void UpdateFromCamera();
    void SetOnCameraChanged(std::function<void()> cb) { onCameraChanged = std::move(cb); }
    
protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnIdle(hui::IdleEvent& evt) override;
    hui::EventResult OnKeyDown(hui::KeyEvent& evt) override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent& evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnKeyUp(hui::KeyEvent& evt) override;

private:
    raytracer::Camera* camera;
    
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
    bool rotateUp = false;    
    bool rotateDown = false;  
    bool yawLeft = false;
    bool yawRight = false;
    bool isCollapsed = false;
    double lastTime = 0.0f;
    
    std::string posXText, posYText, posZText;
    std::string lookAtXText, lookAtYText, lookAtZText;
    std::string fovText;
    
    int activeField = -1;
    std::function<void()> onCameraChanged;

    struct Button {
        ControlPanelIcon icon;
        dr4::Rect2f rect;
        std::function<void(bool)> setState;
    };
    std::vector<Button> buttons;
    int pressedButton = -1;
    int hoverButton = -1;
    int activeButton = -1;
    float titleBarHeight = 26.0f;
    bool dragging = false;
    dr4::Vec2f dragOffset;

    
    dr4::Vec2f expandedSize{0.0f, 0.0f};
    bool hasExpandedSize = false;
    
    void ParseAndApplyChanges();
    void BuildButtons();
    void ResetButtons();
    void ApplyInstantStep(float deltaTime);
    bool ApplyMovementStep(float deltaTime);
    bool pendingStep = false;
    bool pendingKeyStep = false;
    float pendingDelta = 1.0f / 3.0f;
    float moveHoldTime = 0.0f;
    float rotateHoldTime = 0.0f;
    float moveAccumulator = 0.0f;
};

} // namespace ui

#endif // UI_CONTROL_PANEL_HPP