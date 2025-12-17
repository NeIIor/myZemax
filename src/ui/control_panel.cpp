#include "ui/control_panel.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/math/rect.hpp"
#include <algorithm>
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>

namespace ui {

namespace {

static void ClearImage(dr4::Image* img, int w, int h, dr4::Color c) {
    if (!img) return;
    img->SetSize(dr4::Vec2f(static_cast<float>(w), static_cast<float>(h)));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            img->SetPixel(static_cast<size_t>(x), static_cast<size_t>(y), c);
}

static void PutPixelSafe(dr4::Image* img, int w, int h, int x, int y, dr4::Color c) {
    if (!img) return;
    if (x < 0 || y < 0 || x >= w || y >= h) return;
    img->SetPixel(static_cast<size_t>(x), static_cast<size_t>(y), c);
}

static void DrawLine(dr4::Image* img, int w, int h, int x0, int y0, int x1, int y1, dr4::Color c) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        PutPixelSafe(img, w, h, x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void DrawLineN(dr4::Image* img, int w, int h, int x0, int y0, int x1, int y1, dr4::Color c, int thicknessPx) {
    
    thicknessPx = std::clamp(thicknessPx, 1, 3);
    DrawLine(img, w, h, x0, y0, x1, y1, c);
    if (thicknessPx == 1) return;

    int dx = x1 - x0;
    int dy = y1 - y0;
    
    int px = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);
    int py = (dx > 0) ? -1 : (dx < 0 ? 1 : 0);
    
    if (dx == 0 && dy != 0) { px = 1; py = 0; }
    if (dy == 0 && dx != 0) { px = 0; py = 1; }

    DrawLine(img, w, h, x0 + px, y0 + py, x1 + px, y1 + py, c);
    if (thicknessPx == 3) {
        DrawLine(img, w, h, x0 - px, y0 - py, x1 - px, y1 - py, c);
    }
}

static void DrawThickLine(dr4::Image* img, int w, int h, int x0, int y0, int x1, int y1, dr4::Color c) {
    DrawLineN(img, w, h, x0, y0, x1, y1, c, 2);
}

static void DrawCircleOutline(dr4::Image* img, int w, int h, int cx, int cy, int r, dr4::Color c) {
    int x = r;
    int y = 0;
    int err = 0;
    while (x >= y) {
        PutPixelSafe(img, w, h, cx + x, cy + y, c);
        PutPixelSafe(img, w, h, cx + y, cy + x, c);
        PutPixelSafe(img, w, h, cx - y, cy + x, c);
        PutPixelSafe(img, w, h, cx - x, cy + y, c);
        PutPixelSafe(img, w, h, cx - x, cy - y, c);
        PutPixelSafe(img, w, h, cx - y, cy - x, c);
        PutPixelSafe(img, w, h, cx + y, cy - x, c);
        PutPixelSafe(img, w, h, cx + x, cy - y, c);
        ++y;
        if (err <= 0) {
            err += 2 * y + 1;
        } else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

static void DrawFilledCircle(dr4::Image* img, int w, int h, int cx, int cy, int r, dr4::Color c) {
    for (int y = -r; y <= r; ++y) {
        for (int x = -r; x <= r; ++x) {
            if (x * x + y * y <= r * r) {
                PutPixelSafe(img, w, h, cx + x, cy + y, c);
            }
        }
    }
}

static void DrawEllipseOutline(dr4::Image* img,
                               int w,
                               int h,
                               int cx,
                               int cy,
                               float rx,
                               float ry,
                               dr4::Color c,
                               bool dashBackHalf,
                               bool backIsTopHalf) {
    
    
    const int samples = 96;
    auto isBackPt = [&](int px, int py) {
        return backIsTopHalf ? (py < cy) : (px < cx);
    };

    int firstX = 0, firstY = 0;
    bool firstBack = false;
    int prevX = 0, prevY = 0;
    bool prevBack = false;
    bool havePrev = false;

    for (int i = 0; i <= samples; ++i) {
        float t = static_cast<float>(i % samples) / static_cast<float>(samples) * 2.0f * 3.1415926f;
        int px = static_cast<int>(std::lround(static_cast<float>(cx) + std::cos(t) * rx));
        int py = static_cast<int>(std::lround(static_cast<float>(cy) + std::sin(t) * ry));
        bool curBack = isBackPt(px, py);

        if (!havePrev) {
            firstX = px; firstY = py; firstBack = curBack;
            prevX = px; prevY = py; prevBack = curBack;
            havePrev = true;
            continue;
        }

        
        bool segBack = curBack && prevBack;
        int thickness = segBack ? 1 : 2;
        dr4::Color cc = c;
        if (dashBackHalf && segBack) {
            cc = dr4::Color(c.r, c.g, c.b, 140);
        }
        DrawLineN(img, w, h, prevX, prevY, px, py, cc, thickness);

        prevX = px; prevY = py; prevBack = curBack;
    }

    
    if (havePrev) {
        bool segBack = firstBack && prevBack;
        int thickness = segBack ? 1 : 2;
        dr4::Color cc = c;
        if (dashBackHalf && segBack) {
            cc = dr4::Color(c.r, c.g, c.b, 140);
        }
        DrawLineN(img, w, h, prevX, prevY, firstX, firstY, cc, thickness);
    }
}

static void DrawArrowHead(dr4::Image* img, int w, int h, int tipX, int tipY, int dirX, int dirY, dr4::Color c) {
    
    int dx = (dirX > 0) ? 1 : (dirX < 0 ? -1 : 0);
    int dy = (dirY > 0) ? 1 : (dirY < 0 ? -1 : 0);
    if (dx == 0 && dy == 0) return;

    int px = -dy;
    int py = dx;

    auto inside = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < w && y < h;
    };

    
    
    int headLen = 4;
    int spread = 3;

    auto compute = [&](int hl, int sp, int& lx, int& ly, int& rx, int& ry) {
        int bx = tipX - dx * hl;
        int by = tipY - dy * hl;
        lx = bx + px * sp;
        ly = by + py * sp;
        rx = bx - px * sp;
        ry = by - py * sp;
    };

    int lx = 0, ly = 0, rx = 0, ry = 0;
    compute(headLen, spread, lx, ly, rx, ry);
    if (!inside(lx, ly) || !inside(rx, ry)) {
        spread = 2;
        compute(headLen, spread, lx, ly, rx, ry);
    }
    if (!inside(lx, ly) || !inside(rx, ry)) {
        spread = 1;
        compute(headLen, spread, lx, ly, rx, ry);
    }
    if (!inside(lx, ly) || !inside(rx, ry)) {
        headLen = 3;
        compute(headLen, spread, lx, ly, rx, ry);
    }

    PutPixelSafe(img, w, h, tipX, tipY, c);
    DrawLine(img, w, h, tipX, tipY, lx, ly, c);
    DrawLine(img, w, h, tipX, tipY, rx, ry, c);
}

[[maybe_unused]] static void DrawEllipseArc(dr4::Image* img,
                           int w,
                           int h,
                           int cx,
                           int cy,
                           float rx,
                           float ry,
                           float startDeg,
                           float endDeg,
                           int stepSign,
                           dr4::Color c,
                           int thicknessPx = 3) {
    
    
    auto deg2rad = [](float d) { return d * 3.1415926f / 180.0f; };
    auto norm = [](float d) {
        while (d < 0.0f) d += 360.0f;
        while (d >= 360.0f) d -= 360.0f;
        return d;
    };
    float a = norm(startDeg);
    float b = norm(endDeg);

    auto done = [&](float cur) {
        
        if (stepSign > 0) {
            float dist = norm(b - cur);
            return dist < 0.5f;
        } else {
            float dist = norm(cur - b);
            return dist < 0.5f;
        }
    };

    int prevX = 0, prevY = 0;
    bool havePrev = false;
    int safety = 0;
    while (true) {
        float t = deg2rad(a);
        int px = static_cast<int>(std::lround(static_cast<float>(cx) + std::cos(t) * rx));
        int py = static_cast<int>(std::lround(static_cast<float>(cy) + std::sin(t) * ry));
        if (havePrev) {
            DrawLineN(img, w, h, prevX, prevY, px, py, c, thicknessPx);
        }
        prevX = px; prevY = py; havePrev = true;

        if (done(a)) break;
        a = norm(a + static_cast<float>(stepSign) * 5.0f); 
        if (++safety > 80) break;
    }
}

static void DrawCurvedArrowOnEllipse(dr4::Image* img,
                                     int w,
                                     int h,
                                     int cx,
                                     int cy,
                                     float rx,
                                     float ry,
                                     float startDeg,
                                     float endDeg,
                                     int stepSign,
                                     dr4::Color c,
                                     int thicknessPx,
                                     float headClockwiseDeg = 0.0f) {
    
    DrawEllipseArc(img, w, h, cx, cy, rx, ry, startDeg, endDeg, stepSign, c, thicknessPx);

    
    float theta = endDeg * 3.1415926f / 180.0f;
    int tipX = static_cast<int>(std::lround(static_cast<float>(cx) + std::cos(theta) * rx));
    int tipY = static_cast<int>(std::lround(static_cast<float>(cy) + std::sin(theta) * ry));

    
    const float stepDeg = 10.0f; 
    float prevDeg = endDeg - static_cast<float>(stepSign) * stepDeg;
    float ptheta = prevDeg * 3.1415926f / 180.0f;
    int prevX = static_cast<int>(std::lround(static_cast<float>(cx) + std::cos(ptheta) * rx));
    int prevY = static_cast<int>(std::lround(static_cast<float>(cy) + std::sin(ptheta) * ry));

    float vx = static_cast<float>(tipX - prevX);
    float vy = static_cast<float>(tipY - prevY);

    if (headClockwiseDeg != 0.0f) {
        float rad = headClockwiseDeg * 3.1415926f / 180.0f;
        float cs = std::cos(rad);
        float sn = std::sin(rad);
        
        
        float nx = vx * cs + vy * sn;
        float ny = -vx * sn + vy * cs;
        vx = nx;
        vy = ny;
    }

    int dirX = (vx > 0.5f) ? 1 : (vx < -0.5f ? -1 : 0);
    int dirY = (vy > 0.5f) ? 1 : (vy < -0.5f ? -1 : 0);

    
    if (dirX == 0 && dirY == 0) {
        float tx = -std::sin(theta) * rx;
        float ty =  std::cos(theta) * ry;
        if (stepSign < 0) { tx = -tx; ty = -ty; }
        dirX = (tx > 0.0f) ? 1 : (tx < 0.0f ? -1 : 0);
        dirY = (ty > 0.0f) ? 1 : (ty < 0.0f ? -1 : 0);
    }
    DrawArrowHead(img, w, h, tipX, tipY, dirX, dirY, c);
}

static void DrawArrowUp(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    
    DrawLine(img, w, h, cx,     cy + 9, cx,     cy - 7, c);
    DrawLine(img, w, h, cx + 1, cy + 9, cx + 1, cy - 7, c);
    DrawLine(img, w, h, cx,     cy - 7, cx - 6, cy - 1, c);
    DrawLine(img, w, h, cx + 1, cy - 7, cx - 5, cy - 1, c);
    DrawLine(img, w, h, cx,     cy - 7, cx + 6, cy - 1, c);
    DrawLine(img, w, h, cx + 1, cy - 7, cx + 7, cy - 1, c);
}

static void DrawArrowDown(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    DrawLine(img, w, h, cx,     cy - 9, cx,     cy + 7, c);
    DrawLine(img, w, h, cx + 1, cy - 9, cx + 1, cy + 7, c);
    DrawLine(img, w, h, cx,     cy + 7, cx - 6, cy + 1, c);
    DrawLine(img, w, h, cx + 1, cy + 7, cx - 5, cy + 1, c);
    DrawLine(img, w, h, cx,     cy + 7, cx + 6, cy + 1, c);
    DrawLine(img, w, h, cx + 1, cy + 7, cx + 7, cy + 1, c);
}

static void DrawArrowLeft(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    DrawLine(img, w, h, cx + 9, cy,     cx - 7, cy,     c);
    DrawLine(img, w, h, cx + 9, cy + 1, cx - 7, cy + 1, c);
    DrawLine(img, w, h, cx - 7, cy,     cx - 1, cy - 6, c);
    DrawLine(img, w, h, cx - 7, cy + 1, cx - 1, cy - 5, c);
    DrawLine(img, w, h, cx - 7, cy,     cx - 1, cy + 6, c);
    DrawLine(img, w, h, cx - 7, cy + 1, cx - 1, cy + 7, c);
}

static void DrawArrowRight(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    DrawLine(img, w, h, cx - 9, cy,     cx + 7, cy,     c);
    DrawLine(img, w, h, cx - 9, cy + 1, cx + 7, cy + 1, c);
    DrawLine(img, w, h, cx + 7, cy,     cx + 1, cy - 6, c);
    DrawLine(img, w, h, cx + 7, cy + 1, cx + 1, cy - 5, c);
    DrawLine(img, w, h, cx + 7, cy,     cx + 1, cy + 6, c);
    DrawLine(img, w, h, cx + 7, cy + 1, cx + 1, cy + 7, c);
}

static void DrawBullseye(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    DrawCircleOutline(img, w, h, cx, cy, 8, c);
    DrawCircleOutline(img, w, h, cx, cy, 7, c);
    DrawFilledCircle(img, w, h, cx, cy, 2, c);
}

static void DrawCrossInCircle(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    DrawCircleOutline(img, w, h, cx, cy, 8, c);
    DrawCircleOutline(img, w, h, cx, cy, 7, c);
    DrawLineN(img, w, h, cx - 5, cy - 5, cx + 5, cy + 5, c, 2);
    DrawLineN(img, w, h, cx - 5, cy + 5, cx + 5, cy - 5, c, 2);
}


static void DrawRotateYawLeft(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    
    const int ex = cx - 1; 
    const float rx = 8.0f;
    const float ry = 4.5f;
    DrawEllipseOutline(img, w, h, ex, cy, rx, ry, c, true, true);
    DrawEllipseOutline(img, w, h, ex, cy + 1, rx, ry, c, true, true);

    
    DrawCurvedArrowOnEllipse(img, w, h,
                             ex, cy,
                             rx + 10.0f, ry + 10.0f,
                             315.0f, 225.0f,
                             -1, c,
                             1);
}

static void DrawRotateYawRight(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    const int ex = cx - 1; 
    const float rx = 8.0f;
    const float ry = 4.5f;
    DrawEllipseOutline(img, w, h, ex, cy, rx, ry, c, true, true);
    DrawEllipseOutline(img, w, h, ex, cy + 1, rx, ry, c, true, true);

    
    DrawCurvedArrowOnEllipse(img, w, h,
                             ex, cy,
                             rx + 10.0f, ry + 10.0f,
                             225.0f, 315.0f,
                             +1, c,
                             1);
}

static void DrawRotatePitchUp(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    
    
    const int ex = cx;
    const float rx = 5.5f;
    const float ry = 9.0f;
    DrawEllipseOutline(img, w, h, ex, cy, rx, ry, c, true, false);
    DrawEllipseOutline(img, w, h, ex + 1, cy, rx, ry, c, true, false);

    
    DrawCurvedArrowOnEllipse(img, w, h,
                             ex, cy,
                             rx + 7.0f, ry + 7.0f,
                             70.0f, 315.0f,
                             -1, c,
                             1,
                             15.0f);
}

static void DrawRotatePitchDown(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    const int ex = cx;
    const float rx = 5.5f;
    const float ry = 9.0f;
    DrawEllipseOutline(img, w, h, ex, cy, rx, ry, c, true, false);
    DrawEllipseOutline(img, w, h, ex + 1, cy, rx, ry, c, true, false);

    
    DrawCurvedArrowOnEllipse(img, w, h,
                             ex, cy,
                             rx + 7.0f, ry + 7.0f,
                             315.0f, 70.0f,
                             +1, c,
                             1);
}

static void DrawResetIcon(dr4::Image* img, int w, int h, int cx, int cy, dr4::Color c) {
    
    DrawBullseye(img, w, h, cx, cy, c);
    DrawLineN(img, w, h, cx - 12, cy, cx - 7, cy, c, 2);
    DrawLineN(img, w, h, cx + 7, cy, cx + 12, cy, c, 2);
    DrawLineN(img, w, h, cx, cy - 12, cx, cy - 7, c, 2);
    DrawLineN(img, w, h, cx, cy + 7, cx, cy + 12, c, 2);
}

static void DrawControlIconBitmap(dr4::Window* w, dr4::Image* img, ui::ControlPanelIcon icon) {
    (void)w;
    const int W = 32, H = 32;
    const dr4::Color transparent(0, 0, 0, 0);
    const dr4::Color fg(235, 235, 235, 255);
    ClearImage(img, W, H, transparent);
    const int cx = 16, cy = 16;

    switch (icon) {
        case ui::ControlPanelIcon::Forward: DrawBullseye(img, W, H, cx, cy, fg); break; 
        case ui::ControlPanelIcon::Back:    DrawCrossInCircle(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::Left:    DrawArrowLeft(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::Right:   DrawArrowRight(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::Up:      DrawArrowUp(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::Down:    DrawArrowDown(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::YawLeft:  DrawRotateYawLeft(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::YawRight: DrawRotateYawRight(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::PitchUp:  DrawRotatePitchUp(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::PitchDown:DrawRotatePitchDown(img, W, H, cx, cy, fg); break;
        case ui::ControlPanelIcon::ResetCam: DrawResetIcon(img, W, H, cx, cy, fg); break;
    }
}

} // namespace

ControlPanel::ControlPanel(hui::UI* ui, raytracer::Camera* camera_)
    : Container(ui), camera(camera_) {
    SetSize(200, 200);
}

void ControlPanel::Redraw() const {
    dr4::Texture& texture = GetTexture();

    const dr4::Color bgPanel(58, 58, 58);
    const dr4::Color bgHeader(76, 76, 76);
    const dr4::Color borderHeader(96, 96, 96);
    const dr4::Color textMain(235, 235, 235);
    const dr4::Color textSub(210, 210, 210);
    const dr4::Color textAccent(120, 170, 230);
    const dr4::Color btnBase(68, 68, 68);
    const dr4::Color btnHover(88, 98, 110);
    const dr4::Color btnActive(108, 138, 170);

    if (isCollapsed) {
        texture.Clear(dr4::Color(0, 0, 0, 0));
    } else {
        texture.Clear(bgPanel);
    }


    auto* bar = GetUI()->GetWindow()->CreateRectangle();
    bar->SetPos(dr4::Vec2f(0, 0));
    bar->SetSize(dr4::Vec2f(GetSize().x, titleBarHeight));
    bar->SetFillColor(bgHeader);
    bar->SetBorderColor(borderHeader);
    bar->SetBorderThickness(1.0f);
    texture.Draw(*bar);
    
    auto* titleText = GetUI()->GetWindow()->CreateText();
    titleText->SetText("Control Panel");
    titleText->SetPos(dr4::Vec2f(10, 5));
    titleText->SetFontSize(14);
    titleText->SetColor(textMain);
    texture.Draw(*titleText);
    
    auto* arrowText = GetUI()->GetWindow()->CreateText();
    arrowText->SetText(isCollapsed ? ">" : "v");
    arrowText->SetPos(dr4::Vec2f(GetSize().x - 20, 5));
    arrowText->SetFontSize(14);
    arrowText->SetColor(textMain);
    texture.Draw(*arrowText);
    
    if (isCollapsed) return;
    
    float y = titleBarHeight + 8.0f;
    float lineHeight = 22.0f;
    
    auto* text = GetUI()->GetWindow()->CreateText();
    text->SetFontSize(12);
    text->SetColor(textSub);
    
    text->SetText("WASD / Space / Shift - Move");
    text->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*text);
    y += lineHeight;
    
    text->SetText("Arrows - Look/Turn");
    text->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*text);

    y += lineHeight + 8.0f;
    const_cast<ControlPanel*>(this)->BuildButtons();

    for (size_t i = 0; i < buttons.size(); ++i) {
        const auto& b = buttons[i];
        auto* rect = GetUI()->GetWindow()->CreateRectangle();
        rect->SetPos(b.rect.pos);
        rect->SetSize(b.rect.size);
        bool isPressed = static_cast<int>(i) == pressedButton;
        bool isActive = static_cast<int>(i) == activeButton;
        bool isHover = static_cast<int>(i) == hoverButton;
        rect->SetFillColor(isPressed || isActive ? btnActive
                                     : (isHover ? btnHover
                                                : btnBase));
        rect->SetBorderColor(borderHeader);
        rect->SetBorderThickness(1.0f);
        texture.Draw(*rect);

        
        auto* icon = GetUI()->GetWindow()->CreateImage();
        DrawControlIconBitmap(GetUI()->GetWindow(), icon, b.icon);
        float ix = b.rect.pos.x + (b.rect.size.x - 32.0f) * 0.5f;
        float iy = b.rect.pos.y + (b.rect.size.y - 32.0f) * 0.5f;
        icon->SetPos(dr4::Vec2f(ix, iy));
        texture.Draw(*icon);
    }
}

hui::EventResult ControlPanel::PropagateToChildren(hui::Event& event) {
    return hui::EventResult::UNHANDLED;
}

hui::EventResult ControlPanel::OnMouseDown(hui::MouseButtonEvent& evt) {
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (debugInput) {
        std::cout << "[ui] ControlPanel MouseDown pos=(" << evt.pos.x << "," << evt.pos.y << ")\n";
    }
    if (evt.button == dr4::MouseButtonType::LEFT) {

        if (evt.pos.y <= titleBarHeight) {

            if (evt.pos.x > GetSize().x - 30) {
                isCollapsed = !isCollapsed;
                
                if (isCollapsed) {
                    expandedSize = GetSize();
                    hasExpandedSize = true;
                    SetSize(GetSize().x, titleBarHeight);
                } else if (hasExpandedSize) {
                    SetSize(expandedSize);
                }
                ResetButtons();
                ForceRedraw();
                GetUI()->ReportFocus(this);
                return hui::EventResult::HANDLED;
            }

            dragging = true;
            dragOffset = evt.pos;
            GetUI()->ReportFocus(this);
            return hui::EventResult::HANDLED;
        }

        
        if (isCollapsed) {
            return hui::EventResult::UNHANDLED;
        }

        if (!isCollapsed) {
            BuildButtons();
            for (size_t i = 0; i < buttons.size(); ++i) {
                if (buttons[i].rect.Contains(evt.pos)) {
                    pressedButton = static_cast<int>(i);
                    activeButton = pressedButton;
                    buttons[i].setState(true);
                    ApplyInstantStep(1.0f / 30.0f);
                    pendingStep = false;
                    pendingDelta = 1.0f / 30.0f;
                    if (onCameraChanged) {
                        onCameraChanged();
                    }
                    GetUI()->ReportFocus(this);
            ForceRedraw();
                    return hui::EventResult::HANDLED;
                }
            }

            return hui::EventResult::HANDLED;
        }
    }
    return Container::OnMouseDown(evt);
}

hui::EventResult ControlPanel::OnMouseMove(hui::MouseMoveEvent& evt) {
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (dragging) {
        dr4::Vec2f delta = evt.pos - dragOffset;
        SetPos(GetPos() + delta);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (isCollapsed) return hui::EventResult::UNHANDLED;
    BuildButtons();
    int newHover = -1;
    for (size_t i = 0; i < buttons.size(); ++i) {
        if (buttons[i].rect.Contains(evt.pos)) {
            newHover = static_cast<int>(i);
            break;
        }
    }
    if (newHover != hoverButton) {
        hoverButton = newHover;
        ForceRedraw();
    }
    if (debugInput && newHover >= 0) {
        std::cout << "[ui] ControlPanel hover button " << newHover
                  << " pos=(" << evt.pos.x << "," << evt.pos.y << ")\n";
    }
    return Container::OnMouseMove(evt);
}

hui::EventResult ControlPanel::OnKeyDown(hui::KeyEvent& evt) {
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (!camera) return Container::OnKeyDown(evt);
    
    switch (evt.key) {
        case dr4::KEYCODE_W: moveForward = true; break;
        case dr4::KEYCODE_S: moveBackward = true; break;
        case dr4::KEYCODE_A: moveLeft = true; break;
        case dr4::KEYCODE_D: moveRight = true; break;
        case dr4::KEYCODE_SPACE: moveUp = true; break;
        case dr4::KEYCODE_LSHIFT: moveDown = true; break;
        case dr4::KEYCODE_UP: rotateUp = true; break;
        case dr4::KEYCODE_DOWN: rotateDown = true; break;
        case dr4::KEYCODE_LEFT: yawLeft = true; break;
        case dr4::KEYCODE_RIGHT: yawRight = true; break;
        default: return Container::OnKeyDown(evt);
    }
    pendingKeyStep = true;
    pendingDelta = 1.0f / 10.0f;

    if (ApplyMovementStep(pendingDelta) && onCameraChanged) {
        onCameraChanged();
    }
    if (debugInput) {
        std::cout << "[ui] ControlPanel KeyDown code=" << evt.key << "\n";
    }
    return hui::EventResult::HANDLED;
}

hui::EventResult ControlPanel::OnMouseUp(hui::MouseButtonEvent& evt) {
    if (dragging) {
        dragging = false;
        return hui::EventResult::HANDLED;
    }
    if (isCollapsed) {
        return hui::EventResult::UNHANDLED;
    }
    if (pressedButton >= 0 && pressedButton < static_cast<int>(buttons.size())) {
        buttons[pressedButton].setState(false);
    }
    pressedButton = -1;
    activeButton = -1;
    ForceRedraw();
    GetUI()->ReportFocus(this);
    return Container::OnMouseUp(evt);
}

hui::EventResult ControlPanel::OnKeyUp(hui::KeyEvent& evt) {
    static bool debugInput = std::getenv("MYZEMAX_DEBUG_INPUT") != nullptr;
    if (!camera) return Container::OnKeyUp(evt);
    
    switch (evt.key) {
        case dr4::KEYCODE_W: moveForward = false; break;
        case dr4::KEYCODE_S: moveBackward = false; break;
        case dr4::KEYCODE_A: moveLeft = false; break;
        case dr4::KEYCODE_D: moveRight = false; break;
        case dr4::KEYCODE_SPACE: moveUp = false; break;
        case dr4::KEYCODE_LSHIFT: moveDown = false; break;
        case dr4::KEYCODE_UP: rotateUp = false; break;
        case dr4::KEYCODE_DOWN: rotateDown = false; break;
        case dr4::KEYCODE_LEFT: yawLeft = false; break;
        case dr4::KEYCODE_RIGHT: yawRight = false; break;
        default: return Container::OnKeyUp(evt);
    }
    
    if (debugInput) {
        std::cout << "[ui] ControlPanel KeyUp code=" << evt.key << "\n";
    }
    return hui::EventResult::HANDLED;
}

hui::EventResult ControlPanel::OnIdle(hui::IdleEvent& evt) {
    if (!camera) return Container::OnIdle(evt);
    
    float deltaTime = static_cast<float>(evt.deltaTime);
    
    bool changed = false;

    if (pendingKeyStep) {
        changed |= ApplyMovementStep(pendingDelta);
        pendingKeyStep = false;
    }


    moveAccumulator += deltaTime;
    const float step = 1.0f;
    if (moveAccumulator >= step) {
        float dt = step;
        changed |= ApplyMovementStep(dt);
        moveAccumulator -= step;
    }

    if (pendingStep) {
        ApplyInstantStep(pendingDelta);
        pendingStep = false;
        changed = true;
    }

    if (changed && onCameraChanged) {
        onCameraChanged();
    }
    
    lastTime = evt.absTime;
    
    return Container::OnIdle(evt);
}

void ControlPanel::BuildButtons() {
    buttons.clear();

    
    
    
    
    const float startX = 8.0f;
    const float startY = titleBarHeight + 8.0f + 22.0f + 22.0f + 10.0f; 
    const float bw = 64.0f;
    const float bh = 44.0f;
    const float gap = 6.0f;

    auto add = [&](ControlPanelIcon icon, int col, int row, std::function<void(bool)> setter, float spanCols = 1.0f) {
        Button btn;
        btn.icon = icon;
        float w = spanCols * bw + (spanCols - 1.0f) * gap;
        btn.rect = dr4::Rect2f(
            dr4::Vec2f(startX + col * (bw + gap), startY + row * (bh + gap)),
            dr4::Vec2f(w, bh)
        );
        btn.setState = std::move(setter);
        buttons.push_back(std::move(btn));
    };

    add(ControlPanelIcon::Forward,   0, 0, [this](bool v) { moveForward = v; });
    add(ControlPanelIcon::Back,      1, 0, [this](bool v) { moveBackward = v; });
    add(ControlPanelIcon::YawLeft,   2, 0, [this](bool v) { yawLeft = v; });
    add(ControlPanelIcon::YawRight,  3, 0, [this](bool v) { yawRight = v; });

    add(ControlPanelIcon::Left,      0, 1, [this](bool v) { moveLeft = v; });
    add(ControlPanelIcon::Right,     1, 1, [this](bool v) { moveRight = v; });
    add(ControlPanelIcon::PitchUp,   2, 1, [this](bool v) { rotateUp = v; });
    add(ControlPanelIcon::PitchDown, 3, 1, [this](bool v) { rotateDown = v; });

    add(ControlPanelIcon::Up,        0, 2, [this](bool v) { moveUp = v; });
    add(ControlPanelIcon::Down,      1, 2, [this](bool v) { moveDown = v; });
    add(ControlPanelIcon::ResetCam,  2, 2, [this](bool v) {
        if (!v || !camera) return;
        camera->position = raytracer::Vec3(0, 0, 5);
        camera->target = raytracer::Vec3(0, 0, 0);
        camera->currentMoveSpeed = 0.0f;
        camera->currentRotationSpeed = 0.0f;
        if (onCameraChanged) onCameraChanged();
    }, 2.0f);
}

void ControlPanel::ResetButtons() {
    moveForward = moveBackward = moveLeft = moveRight = moveUp = moveDown = false;
    rotateUp = rotateDown = false;
    yawLeft = yawRight = false;
    pressedButton = -1;
    activeButton = -1;
}

void ControlPanel::ApplyInstantStep(float deltaTime) {
    if (!camera) return;

    bool moved = false;
    bool rotated = false;

    const float moveStep = camera->moveSpeed * deltaTime;
    const float rotStep  = camera->rotationSpeed * deltaTime;

    if (pressedButton >= 0 && pressedButton < static_cast<int>(buttons.size())) {
        switch (buttons[pressedButton].icon) {
            case ControlPanelIcon::Forward:
                camera->position += camera->GetForward() * moveStep; camera->target += camera->GetForward() * moveStep; moved = true; break;
            case ControlPanelIcon::Back:
                camera->position -= camera->GetForward() * moveStep; camera->target -= camera->GetForward() * moveStep; moved = true; break;
            case ControlPanelIcon::Left:
                camera->position -= camera->GetRight() * moveStep;   camera->target -= camera->GetRight() * moveStep;   moved = true; break;
            case ControlPanelIcon::Right:
                camera->position += camera->GetRight() * moveStep;   camera->target += camera->GetRight() * moveStep;   moved = true; break;
            case ControlPanelIcon::Up:
                camera->position += camera->GetUp() * moveStep;      camera->target += camera->GetUp() * moveStep;      moved = true; break;
            case ControlPanelIcon::Down:
                camera->position -= camera->GetUp() * moveStep;      camera->target -= camera->GetUp() * moveStep;      moved = true; break;
            case ControlPanelIcon::YawLeft:
                camera->RotateYaw(-rotStep); rotated = true; break;
            case ControlPanelIcon::YawRight:
                camera->RotateYaw(rotStep); rotated = true; break;
            case ControlPanelIcon::PitchUp:
                camera->RotatePitch(rotStep); rotated = true; break;
            case ControlPanelIcon::PitchDown:
                camera->RotatePitch(-rotStep); rotated = true; break;
            case ControlPanelIcon::ResetCam:
                
                break;
        }
    }

    moveForward = moveBackward = moveLeft = moveRight = moveUp = moveDown = false;
    rotateUp = rotateDown = false;
    yawLeft = yawRight = false;

    if (moved || rotated) {
        if (onCameraChanged) onCameraChanged();
    }
}

bool ControlPanel::ApplyMovementStep(float deltaTime) {
    if (!camera) return false;

    bool isMoving = moveForward || moveBackward || moveLeft || moveRight || moveUp || moveDown;
    bool isRotating = rotateUp || rotateDown || yawLeft || yawRight;


    if (isMoving) moveHoldTime = std::min(moveHoldTime + deltaTime, 9.0f);
    else moveHoldTime = 0.0f;
    if (isRotating) rotateHoldTime = std::min(rotateHoldTime + deltaTime, 9.0f);
    else rotateHoldTime = 0.0f;

    float moveRamp = (moveHoldTime <= 3.0f) ? 1.0f
                    : (1.0f + 1.0f * ((moveHoldTime - 3.0f) / 6.0f));
    float rotRamp  = (rotateHoldTime <= 3.0f) ? 1.0f
                    : (1.0f + 1.0f * ((rotateHoldTime - 3.0f) / 6.0f));


    float usedDt = std::min(deltaTime, 1.0f / 30.0f);

    camera->UpdateSpeed(usedDt, isMoving);
    camera->UpdateRotationSpeed(usedDt, isRotating);

    bool changed = false;
    
    if (moveForward) { camera->MoveForward(usedDt * moveRamp); changed = true; }
    if (moveBackward) { camera->MoveBackward(usedDt * moveRamp); changed = true; }
    if (moveLeft) { camera->MoveLeft(usedDt * moveRamp); changed = true; }
    if (moveRight) { camera->MoveRight(usedDt * moveRamp); changed = true; }
    if (moveUp) { camera->MoveUp(usedDt * moveRamp); changed = true; }
    if (moveDown) { camera->MoveDown(usedDt * moveRamp); changed = true; }
    
    const float rotScale = 0.3f;
    if (rotateUp) { camera->RotatePitch(camera->currentRotationSpeed * usedDt * rotRamp * rotScale); changed = true; }
    if (rotateDown) { camera->RotatePitch(-camera->currentRotationSpeed * usedDt * rotRamp * rotScale); changed = true; }
    if (yawLeft) { camera->RotateYaw(-camera->currentRotationSpeed * usedDt * rotRamp * rotScale); changed = true; }
    if (yawRight) { camera->RotateYaw(camera->currentRotationSpeed * usedDt * rotRamp * rotScale); changed = true; }

    return changed;
}

} // namespace ui