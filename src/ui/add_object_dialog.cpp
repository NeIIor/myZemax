#include "ui/add_object_dialog.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/keycodes.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include <algorithm>

namespace ui {

AddObjectDialog::AddObjectDialog(hui::UI* ui) : Container(ui) {
    SetSize(220, 180);
    items = {
        {"Sphere", 0},
        {"Plane", 1},
        {"Bounded plane", 6},
        {"Disk", 2},
        {"Prism", 3},
        {"Pyramid", 4},
        {"Light", 5},
    };
}

void AddObjectDialog::Show() {
    visible = true;
    dragging = false;
    draggingThumb = false;
    hoverIndex = -1;
    hoverOk = hoverCancel = false;
    pressedOk = pressedCancel = false;
    ClampScroll();
    ForceRedraw();
}

void AddObjectDialog::Hide() {
    visible = false;
    dragging = false;
    draggingThumb = false;
    hoverIndex = -1;
    hoverOk = hoverCancel = false;
    pressedOk = pressedCancel = false;
    ForceRedraw();
}

hui::EventResult AddObjectDialog::PropagateToChildren(hui::Event&) {
    return hui::EventResult::UNHANDLED;
}

void AddObjectDialog::ClampScroll() {
    float listY = titleBarHeight + 10.0f;
    float listH = GetSize().y - listY - 46.0f;
    float itemH = 24.0f;
    float contentH = static_cast<float>(items.size()) * itemH;
    float maxScroll = std::max(0.0f, contentH - listH);
    scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);
}

void AddObjectDialog::Redraw() const {
    dr4::Texture& texture = GetTexture();
    if (!visible) {
        texture.Clear(dr4::Color(0, 0, 0, 0));
        return;
    }

    const dr4::Color bg(56, 56, 60);
    const dr4::Color hdr(76, 76, 76);
    const dr4::Color border(96, 96, 96);
    const dr4::Color textMain(235, 235, 235);
    const dr4::Color textSub(210, 210, 210);
    const dr4::Color itemHover(78, 88, 100);
    const dr4::Color itemSel(120, 170, 230, 120);
    const dr4::Color btnBase(70, 70, 72);
    const dr4::Color btnHover(88, 88, 92);
    const dr4::Color btnPressed(62, 62, 64);

    texture.Clear(bg);

    auto* bar = GetUI()->GetWindow()->CreateRectangle();
    bar->SetPos(dr4::Vec2f(0, 0));
    bar->SetSize(dr4::Vec2f(GetSize().x, titleBarHeight));
    bar->SetFillColor(hdr);
    bar->SetBorderColor(border);
    bar->SetBorderThickness(1.0f);
    texture.Draw(*bar);

    auto* title = GetUI()->GetWindow()->CreateText();
    title->SetText("Add object");
    title->SetPos(dr4::Vec2f(10, 6));
    title->SetFontSize(14);
    title->SetColor(textMain);
    texture.Draw(*title);

    auto* closeText = GetUI()->GetWindow()->CreateText();
    closeText->SetText("x");
    closeText->SetPos(dr4::Vec2f(GetSize().x - 16, 6));
    closeText->SetFontSize(14);
    closeText->SetColor(textSub);
    texture.Draw(*closeText);

    float listX = 10.0f;
    float listY = titleBarHeight + 10.0f;
    float listW = GetSize().x - 20.0f;
    float listH = GetSize().y - listY - 46.0f;
    dr4::Rect2f listRect(dr4::Vec2f(listX, listY), dr4::Vec2f(listW, listH));

    auto* listBg = GetUI()->GetWindow()->CreateRectangle();
    listBg->SetPos(listRect.pos);
    listBg->SetSize(listRect.size);
    listBg->SetFillColor(dr4::Color(48, 48, 52));
    listBg->SetBorderColor(border);
    listBg->SetBorderThickness(1.0f);
    texture.Draw(*listBg);

    float itemH = 24.0f;
    float y = listY - scrollOffset;
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        float iy = y + i * itemH;
        
        
        if (iy < listY || (iy + itemH) > (listY + listH)) continue;

        dr4::Rect2f itemRect(dr4::Vec2f(listX + 2, iy), dr4::Vec2f(listW - 16, itemH));
        if (i == selectedIndex) {
            auto* r = GetUI()->GetWindow()->CreateRectangle();
            r->SetPos(itemRect.pos);
            r->SetSize(itemRect.size);
            r->SetFillColor(itemSel);
            texture.Draw(*r);
        } else if (i == hoverIndex) {
            auto* r = GetUI()->GetWindow()->CreateRectangle();
            r->SetPos(itemRect.pos);
            r->SetSize(itemRect.size);
            r->SetFillColor(itemHover);
            texture.Draw(*r);
        }

        auto* t = GetUI()->GetWindow()->CreateText();
        t->SetText(items[i].label);
        t->SetPos(dr4::Vec2f(itemRect.pos.x + 8, itemRect.pos.y + 4));
        t->SetFontSize(12);
        t->SetColor(textMain);
        texture.Draw(*t);
    }

    float contentH = static_cast<float>(items.size()) * itemH;
    float maxScroll = std::max(0.0f, contentH - listH);
    float trackX = listX + listW - 12.0f;
    float trackY = listY + 2.0f;
    float trackH = listH - 4.0f;
    auto* track = GetUI()->GetWindow()->CreateRectangle();
    track->SetPos(dr4::Vec2f(trackX, trackY));
    track->SetSize(dr4::Vec2f(10.0f, trackH));
    track->SetFillColor(dr4::Color(42, 42, 46));
    track->SetBorderColor(border);
    track->SetBorderThickness(1.0f);
    texture.Draw(*track);

    if (maxScroll > 0.0f) {
        float thumbH = std::max(18.0f, trackH * (listH / contentH));
        float t = (maxScroll <= 0.0f) ? 0.0f : (scrollOffset / maxScroll);
        float thumbY = trackY + t * (trackH - thumbH);
        auto* thumb = GetUI()->GetWindow()->CreateRectangle();
        thumb->SetPos(dr4::Vec2f(trackX + 1.0f, thumbY));
        thumb->SetSize(dr4::Vec2f(8.0f, thumbH));
        thumb->SetFillColor(dr4::Color(110, 110, 116));
        texture.Draw(*thumb);
    }

    float btnY = GetSize().y - 32.0f;
    dr4::Rect2f okRect(dr4::Vec2f(GetSize().x - 140.0f, btnY), dr4::Vec2f(60.0f, 22.0f));
    dr4::Rect2f cancelRect(dr4::Vec2f(GetSize().x - 74.0f, btnY), dr4::Vec2f(60.0f, 22.0f));

    auto drawBtn = [&](const dr4::Rect2f& r, const char* cap, bool hov, bool prs) {
        dr4::Color fill = btnBase;
        if (prs) fill = btnPressed;
        else if (hov) fill = btnHover;
        auto* b = GetUI()->GetWindow()->CreateRectangle();
        b->SetPos(r.pos);
        b->SetSize(r.size);
        b->SetFillColor(fill);
        b->SetBorderColor(border);
        b->SetBorderThickness(1.0f);
        texture.Draw(*b);
        auto* t = GetUI()->GetWindow()->CreateText();
        t->SetText(cap);
        t->SetPos(dr4::Vec2f(r.pos.x + 14.0f, r.pos.y + (prs ? 5.0f : 4.0f)));
        t->SetFontSize(12);
        t->SetColor(textMain);
        texture.Draw(*t);
    };

    drawBtn(okRect, "OK", hoverOk, pressedOk);
    drawBtn(cancelRect, "Cancel", hoverCancel, pressedCancel);
}

hui::EventResult AddObjectDialog::OnMouseDown(hui::MouseButtonEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    if (evt.button != dr4::MouseButtonType::LEFT) return hui::EventResult::HANDLED;

    if (evt.pos.y <= titleBarHeight) {
        if (evt.pos.x >= GetSize().x - 26.0f) {
            if (onCancel) onCancel();
            Hide();
            return hui::EventResult::HANDLED;
        }
        dragging = true;
        dragOffset = evt.pos;
        GetUI()->ReportFocus(this);
        return hui::EventResult::HANDLED;
    }

    float listX = 10.0f;
    float listY = titleBarHeight + 10.0f;
    float listW = GetSize().x - 20.0f;
    float listH = GetSize().y - listY - 46.0f;
    dr4::Rect2f listRect(dr4::Vec2f(listX, listY), dr4::Vec2f(listW, listH));

    float btnY = GetSize().y - 32.0f;
    dr4::Rect2f okRect(dr4::Vec2f(GetSize().x - 140.0f, btnY), dr4::Vec2f(60.0f, 22.0f));
    dr4::Rect2f cancelRect(dr4::Vec2f(GetSize().x - 74.0f, btnY), dr4::Vec2f(60.0f, 22.0f));

    if (okRect.Contains(evt.pos)) {
        pressedOk = true;
        ForceRedraw();
        GetUI()->ReportFocus(this);
        return hui::EventResult::HANDLED;
    }
    if (cancelRect.Contains(evt.pos)) {
        pressedCancel = true;
        ForceRedraw();
        GetUI()->ReportFocus(this);
        return hui::EventResult::HANDLED;
    }

    float itemH = 24.0f;
    if (listRect.Contains(evt.pos)) {
        float trackX = listX + listW - 12.0f;
        float trackY = listY + 2.0f;
        float trackH = listH - 4.0f;
        dr4::Rect2f trackRect(dr4::Vec2f(trackX, trackY), dr4::Vec2f(10.0f, trackH));
        if (trackRect.Contains(evt.pos)) {
            float contentH = static_cast<float>(items.size()) * itemH;
            float maxScroll = std::max(0.0f, contentH - listH);
            if (maxScroll > 0.0f) {
                float thumbH = std::max(18.0f, trackH * (listH / contentH));
                float t = (maxScroll <= 0.0f) ? 0.0f : (scrollOffset / maxScroll);
                float thumbY = trackY + t * (trackH - thumbH);
                dr4::Rect2f thumbRect(dr4::Vec2f(trackX + 1.0f, thumbY), dr4::Vec2f(8.0f, thumbH));
                if (thumbRect.Contains(evt.pos)) {
                    draggingThumb = true;
                    thumbDragOffsetY = evt.pos.y - thumbY;
                    GetUI()->ReportFocus(this);
                    return hui::EventResult::HANDLED;
                }
            }
        }

        int idx = static_cast<int>((evt.pos.y - listY + scrollOffset) / itemH);
        if (idx >= 0 && idx < static_cast<int>(items.size())) {
            selectedIndex = idx;
            ForceRedraw();
            GetUI()->ReportFocus(this);
            return hui::EventResult::HANDLED;
        }
    }

    GetUI()->ReportFocus(this);
    return hui::EventResult::HANDLED;
}

hui::EventResult AddObjectDialog::OnMouseMove(hui::MouseMoveEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    GetUI()->ReportHover(this);

    if (dragging) {
        dr4::Vec2f delta = evt.pos - dragOffset;
        SetPos(GetPos() + delta);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }

    float listX = 10.0f;
    float listY = titleBarHeight + 10.0f;
    float listW = GetSize().x - 20.0f;
    float listH = GetSize().y - listY - 46.0f;
    dr4::Rect2f listRect(dr4::Vec2f(listX, listY), dr4::Vec2f(listW, listH));

    float btnY = GetSize().y - 32.0f;
    dr4::Rect2f okRect(dr4::Vec2f(GetSize().x - 140.0f, btnY), dr4::Vec2f(60.0f, 22.0f));
    dr4::Rect2f cancelRect(dr4::Vec2f(GetSize().x - 74.0f, btnY), dr4::Vec2f(60.0f, 22.0f));

    hoverOk = okRect.Contains(evt.pos);
    hoverCancel = cancelRect.Contains(evt.pos);

    if (draggingThumb) {
        float itemH = 24.0f;
        float contentH = static_cast<float>(items.size()) * itemH;
        float maxScroll = std::max(0.0f, contentH - listH);
        float trackY = listY + 2.0f;
        float trackH = listH - 4.0f;
        float thumbH = std::max(18.0f, trackH * (listH / contentH));
        float localY = std::clamp(evt.pos.y - thumbDragOffsetY, trackY, trackY + trackH - thumbH);
        float t = (trackH - thumbH <= 0.0f) ? 0.0f : (localY - trackY) / (trackH - thumbH);
        scrollOffset = t * maxScroll;
        ClampScroll();
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }

    int newHover = -1;
    float itemH = 24.0f;
    if (listRect.Contains(evt.pos)) {
        int idx = static_cast<int>((evt.pos.y - listY + scrollOffset) / itemH);
        if (idx >= 0 && idx < static_cast<int>(items.size())) newHover = idx;
    }
    if (newHover != hoverIndex) {
        hoverIndex = newHover;
        ForceRedraw();
    } else if (hoverOk || hoverCancel) {
        ForceRedraw();
    }

    return hui::EventResult::HANDLED;
}

hui::EventResult AddObjectDialog::OnMouseWheel(hui::MouseWheelEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    scrollOffset -= evt.delta.y * 20.0f;
    ClampScroll();
    ForceRedraw();
    return hui::EventResult::HANDLED;
}

hui::EventResult AddObjectDialog::OnMouseUp(hui::MouseButtonEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    if (dragging) {
        dragging = false;
        return hui::EventResult::HANDLED;
    }
    if (draggingThumb) {
        draggingThumb = false;
        return hui::EventResult::HANDLED;
    }
    if (evt.button != dr4::MouseButtonType::LEFT) return hui::EventResult::HANDLED;

    float btnY = GetSize().y - 32.0f;
    dr4::Rect2f okRect(dr4::Vec2f(GetSize().x - 140.0f, btnY), dr4::Vec2f(60.0f, 22.0f));
    dr4::Rect2f cancelRect(dr4::Vec2f(GetSize().x - 74.0f, btnY), dr4::Vec2f(60.0f, 22.0f));

    bool fireOk = pressedOk && okRect.Contains(evt.pos);
    bool fireCancel = pressedCancel && cancelRect.Contains(evt.pos);

    pressedOk = false;
    pressedCancel = false;
    ForceRedraw();

    if (fireOk) {
        if (selectedIndex < 0 || selectedIndex >= static_cast<int>(items.size())) return hui::EventResult::HANDLED;
        if (onOk) onOk(items[selectedIndex].kind);
        Hide();
        return hui::EventResult::HANDLED;
    }
    if (fireCancel) {
        if (onCancel) onCancel();
        Hide();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::HANDLED;
}

hui::EventResult AddObjectDialog::OnKeyDown(hui::KeyEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    if (evt.key == dr4::KEYCODE_ESCAPE) {
        if (onCancel) onCancel();
        Hide();
        return hui::EventResult::HANDLED;
    }
    if (evt.key == dr4::KEYCODE_ENTER) {
        if (selectedIndex < 0 || selectedIndex >= static_cast<int>(items.size())) return hui::EventResult::HANDLED;
        if (onOk) onOk(items[selectedIndex].kind);
        Hide();
        return hui::EventResult::HANDLED;
    }
    if (evt.key == dr4::KEYCODE_UP) {
        selectedIndex = std::max(0, selectedIndex - 1);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (evt.key == dr4::KEYCODE_DOWN) {
        selectedIndex = std::min(static_cast<int>(items.size()) - 1, selectedIndex + 1);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::HANDLED;
}

} // namespace ui


