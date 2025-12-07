#include "ui/control_panel.hpp"
#include "dr4/keycodes.hpp"
#include <algorithm>

namespace ui {

ControlPanel::ControlPanel(hui::UI* ui, raytracer::Camera* camera_)
    : Container(ui), camera(camera_), lastTime(0.0f), isCollapsed(false) {
    SetSize(200, 200);
    SetupButtons();
}

void ControlPanel::SetupButtons() {
}

void ControlPanel::Redraw() const {
    dr4::Texture& texture = GetTexture();
    texture.Clear(dr4::Color(50, 50, 50));
    
    auto* titleText = GetWindow()->CreateText();
    titleText->SetText("Camera Control");
    titleText->SetPos(dr4::Vec2f(10, 5));
    titleText->SetFontSize(14);
    titleText->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*titleText);
    
    auto* arrowText = GetWindow()->CreateText();
    arrowText->SetText(isCollapsed ? "▼" : "▶");
    arrowText->SetPos(dr4::Vec2f(GetSize().x - 20, 5));
    arrowText->SetFontSize(14);
    arrowText->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*arrowText);
    
    if (isCollapsed) return;
    
    float y = 30.0f;
    float lineHeight = 20.0f;
    
    auto* text = GetWindow()->CreateText();
    text->SetFontSize(11);
    text->SetColor(dr4::Color(200, 200, 200));
    
    text->SetText("WASD - Move");
    text->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*text);
    y += lineHeight;
    
    text->SetText("Space/Shift - Up/Down");
    text->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*text);
    y += lineHeight;
    
    text->SetText("Arrows - Rotate");
    text->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*text);
}

EventResult ControlPanel::OnMouseDown(MouseButtonEvent& evt) {
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (evt.pos.x > GetSize().x - 30 && evt.pos.y < 25) {
            isCollapsed = !isCollapsed;
            ForceRedraw();
            return EventResult::HANDLED;
        }
    }
    return Container::OnMouseDown(evt);
}

EventResult ControlPanel::OnKeyDown(KeyEvent& evt) {
    if (!camera) return Container::OnKeyDown(evt);
    
    switch (evt.key) {
        case dr4::KEYCODE_W: moveForward = true; break;
        case dr4::KEYCODE_S: moveBackward = true; break;
        case dr4::KEYCODE_A: moveLeft = true; break;
        case dr4::KEYCODE_D: moveRight = true; break;
        case dr4::KEYCODE_SPACE: moveUp = true; break;
        case dr4::KEYCODE_LSHIFT: moveDown = true; break;
        case dr4::KEYCODE_LEFT: rotateLeft = true; break;
        case dr4::KEYCODE_RIGHT: rotateRight = true; break;
        case dr4::KEYCODE_UP: rotateUp = true; break;
        case dr4::KEYCODE_DOWN: rotateDown = true; break;
        default: return Container::OnKeyDown(evt);
    }
    
    return EventResult::HANDLED;
}

EventResult ControlPanel::OnKeyUp(KeyEvent& evt) {
    if (!camera) return Container::OnKeyUp(evt);
    
    switch (evt.key) {
        case dr4::KEYCODE_W: moveForward = false; break;
        case dr4::KEYCODE_S: moveBackward = false; break;
        case dr4::KEYCODE_A: moveLeft = false; break;
        case dr4::KEYCODE_D: moveRight = false; break;
        case dr4::KEYCODE_SPACE: moveUp = false; break;
        case dr4::KEYCODE_LSHIFT: moveDown = false; break;
        case dr4::KEYCODE_LEFT: rotateLeft = false; break;
        case dr4::KEYCODE_RIGHT: rotateRight = false; break;
        case dr4::KEYCODE_UP: rotateUp = false; break;
        case dr4::KEYCODE_DOWN: rotateDown = false; break;
        default: return Container::OnKeyUp(evt);
    }
    
    return EventResult::HANDLED;
}

EventResult ControlPanel::OnIdle(IdleEvent& evt) {
    if (!camera) return Container::OnIdle(evt);
    
    float deltaTime = static_cast<float>(evt.deltaTime);
    
    bool isMoving = moveForward || moveBackward || moveLeft || moveRight || moveUp || moveDown;
    camera->UpdateSpeed(deltaTime, isMoving);
    
    bool isRotating = rotateLeft || rotateRight || rotateUp || rotateDown;
    camera->UpdateRotationSpeed(deltaTime, isRotating);
    
    if (moveForward) camera->MoveForward(deltaTime);
    if (moveBackward) camera->MoveBackward(deltaTime);
    if (moveLeft) camera->MoveLeft(deltaTime);
    if (moveRight) camera->MoveRight(deltaTime);
    if (moveUp) camera->MoveUp(deltaTime);
    if (moveDown) camera->MoveDown(deltaTime);
    
    if (rotateLeft) camera->RotateYaw(-camera->currentRotationSpeed * deltaTime);
    if (rotateRight) camera->RotateYaw(camera->currentRotationSpeed * deltaTime);
    if (rotateUp) camera->RotatePitch(camera->currentRotationSpeed * deltaTime);
    if (rotateDown) camera->RotatePitch(-camera->currentRotationSpeed * deltaTime);
    
    lastTime = evt.absTime;
    
    return Container::OnIdle(evt);
}

} // namespace ui

