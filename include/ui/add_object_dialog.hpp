#ifndef UI_ADD_OBJECT_DIALOG_HPP
#define UI_ADD_OBJECT_DIALOG_HPP

#include "hui/container.hpp"
#include "dr4/math/vec2.hpp"
#include <functional>
#include <vector>
#include <string>

namespace hui {
    class MouseButtonEvent;
    class MouseMoveEvent;
    class MouseWheelEvent;
    class KeyEvent;
}

namespace ui {

class AddObjectDialog : public hui::Container {
public:
    explicit AddObjectDialog(hui::UI* ui);

    void Show();
    void Hide();
    bool IsVisible() const { return visible; }

    void SetOnOk(std::function<void(int kind)> cb) { onOk = std::move(cb); }
    void SetOnCancel(std::function<void()> cb) { onCancel = std::move(cb); }

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent& evt) override;
    hui::EventResult OnMouseWheel(hui::MouseWheelEvent& evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnKeyDown(hui::KeyEvent& evt) override;

private:
    struct Item { std::string label; int kind; };

    bool visible = false;
    float titleBarHeight = 26.0f;
    bool dragging = false;
    dr4::Vec2f dragOffset;

    std::vector<Item> items;
    int selectedIndex = 0;

    float scrollOffset = 0.0f;
    bool draggingThumb = false;
    float thumbDragOffsetY = 0.0f;

    int hoverIndex = -1;
    bool hoverOk = false;
    bool hoverCancel = false;
    bool pressedOk = false;
    bool pressedCancel = false;

    std::function<void(int kind)> onOk;
    std::function<void()> onCancel;

    void ClampScroll();
};

} // namespace ui

#endif // UI_ADD_OBJECT_DIALOG_HPP




