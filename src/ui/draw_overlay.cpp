#include "ui/draw_overlay.hpp"
#include "hui/ui.hpp"
#include "dr4/window.hpp"
#include "dr4/event.hpp"
#include <algorithm>
#include <functional>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <unordered_set>


#include "/home/nellor/c++/dorisovka_pp/include/geom_prim.hpp"

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

static void DrawRectOutline(dr4::Image* img, int w, int h, int x0, int y0, int x1, int y1, dr4::Color c) {
    for (int x = x0; x <= x1; ++x) { PutPixelSafe(img, w, h, x, y0, c); PutPixelSafe(img, w, h, x, y1, c); }
    for (int y = y0; y <= y1; ++y) { PutPixelSafe(img, w, h, x0, y, c); PutPixelSafe(img, w, h, x1, y, c); }
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

static void DrawToolIconBitmap(dr4::Window* w, dr4::Image* img, std::string_view toolName) {
    (void)w;
    const int W = 24, H = 24;
    const dr4::Color transparent(0, 0, 0, 0);
    const dr4::Color fg(235, 235, 235, 255);
    ClearImage(img, W, H, transparent);

    std::string n(toolName);
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char ch){ return static_cast<char>(std::tolower(ch)); });

    if (n.find("rectangle") != std::string::npos) {
        DrawRectOutline(img, W, H, 5, 6, 18, 17, fg);
        DrawRectOutline(img, W, H, 6, 7, 17, 16, fg);
    } else if (n.find("circle") != std::string::npos) {
        DrawCircleOutline(img, W, H, 12, 12, 7, fg);
        DrawCircleOutline(img, W, H, 12, 12, 6, fg);
    } else if (n.find("arrow") != std::string::npos) {
        DrawLine(img, W, H, 6, 16, 18, 8, fg);
        DrawLine(img, W, H, 18, 8, 16, 8, fg);
        DrawLine(img, W, H, 18, 8, 18, 10, fg);
        DrawLine(img, W, H, 18, 8, 16, 10, fg);
    } else if (n.find("line") != std::string::npos) {
        DrawLine(img, W, H, 6, 17, 18, 7, fg);
        DrawLine(img, W, H, 6, 16, 18, 6, fg);
    } else if (n.find("text") != std::string::npos) {
        DrawLine(img, W, H, 7, 7, 17, 7, fg);
        DrawLine(img, W, H, 12, 7, 12, 18, fg);
        DrawLine(img, W, H, 11, 7, 11, 18, fg);
    } else if (n.find("heart") != std::string::npos) {
        
        DrawLine(img, W, H, 12, 18, 6, 12, fg);
        DrawLine(img, W, H, 12, 18, 18, 12, fg);
        DrawCircleOutline(img, W, H, 9, 11, 3, fg);
        DrawCircleOutline(img, W, H, 15, 11, 3, fg);
    } else if (n.find("brush") != std::string::npos) {
        DrawLine(img, W, H, 6, 18, 18, 6, fg);
        DrawLine(img, W, H, 7, 18, 18, 7, fg);
        DrawLine(img, W, H, 6, 17, 17, 6, fg);
        DrawRectOutline(img, W, H, 5, 18, 9, 21, fg);
    } else if (n.find("image") != std::string::npos) {
        DrawRectOutline(img, W, H, 5, 6, 18, 17, fg);
        DrawLine(img, W, H, 6, 16, 10, 12, fg);
        DrawLine(img, W, H, 10, 12, 13, 15, fg);
        DrawLine(img, W, H, 13, 15, 17, 11, fg);
        DrawCircleOutline(img, W, H, 15, 9, 2, fg);
    } else if (n.find("print") != std::string::npos || n.find("screen") != std::string::npos) {
        DrawRectOutline(img, W, H, 6, 8, 17, 16, fg);
        DrawRectOutline(img, W, H, 8, 6, 15, 8, fg);
        DrawLine(img, W, H, 9, 6, 9, 5, fg);
        DrawLine(img, W, H, 14, 6, 14, 5, fg);
        DrawCircleOutline(img, W, H, 12, 12, 3, fg);
    } else {
        
        DrawCircleOutline(img, W, H, 12, 12, 3, fg);
    }
}

} // namespace


struct DrawOverlay::CanvasImpl : public pp::Canvas {
    explicit CanvasImpl(dr4::Window* w, DrawOverlay::StyleConfig* s) : window(w), style(s) {}
    std::vector<std::unique_ptr<pp::Shape>> shapes;
    pp::Shape* selected = nullptr;
    DrawOverlay::StyleConfig* style;
    pp::ControlsTheme GetControlsTheme() const override {
        return pp::ControlsTheme{
            style ? style->fillColor : dr4::Color(120, 170, 230, 60),
            style ? style->strokeColor : dr4::Color(120, 170, 230, 200),
            dr4::Color(255, 200, 120),
            dr4::Color(235, 235, 235),
            14.0f,
            dr4::Color(200, 200, 200),
            dr4::Color(255, 220, 160),
            dr4::Color(120, 170, 230)
        };
    }
    void AddShape(pp::Shape* s) override {
        if (!s) return;
        shapes.emplace_back(s);
    }
    void DelShape(pp::Shape* s) override {
        if (!s) return;
        shapes.erase(std::remove_if(shapes.begin(), shapes.end(),
            [&](const std::unique_ptr<pp::Shape>& ptr){ return ptr.get() == s; }), shapes.end());
        if (selected == s) selected = nullptr;
    }
    void SetSelectedShape(pp::Shape* s) override { selected = s; }
    pp::Shape* GetSelectedShape() const override { return selected; }
    void ShapeChanged(pp::Shape*) override {}
    dr4::Window* GetWindow() override { return window; }
    dr4::Window* window;
};

DrawOverlay::DrawOverlay(hui::UI* ui) : Container(ui) {
    SetSize(200, 200);
    defaultStyle = style;
    BuildSliders();
}

void DrawOverlay::ResetTools() {
    if (activeTool >= 0 && activeTool < static_cast<int>(tools.size()) && tools[activeTool].tool) {
        tools[activeTool].tool->OnEnd();
    }
    tools.clear();
    activeTool = -1;
    settingsVisible = false;
    hoveredTool = -1;
    draggingSlider = -1;
    editingSlider = -1;
    sliders.clear();
    canvas.reset();
    ForceRedraw();
}

void DrawOverlay::SetToolsPluginIdFilter(std::string id) {
    if (toolsPluginIdFilter == id) return;
    toolsPluginIdFilter = std::move(id);
    ResetTools();
}

void DrawOverlay::EnsureLoaded() {
    if (Loaded() || !pluginManager || !GetUI() || !GetUI()->GetWindow()) return;
    SetSize(GetUI()->GetWindow()->GetSize());
    canvas = std::make_unique<CanvasImpl>(GetUI()->GetWindow(), &style);
    const auto& plugins = pluginManager->GetAll();
    
    
    std::unordered_set<std::string> seenIds;
    for (const auto& up : plugins) {
        auto* p = up.get();
        if (auto* pp = dynamic_cast<cum::PPToolPlugin*>(p)) {
            std::string id(p->GetIdentifier());
            if (!toolsPluginIdFilter.empty() && id != toolsPluginIdFilter) {
                continue;
            }
            if (!seenIds.insert(id).second) {
                continue;
            }
            auto vec = pp->CreateTools(canvas.get());
            for (auto& t : vec) {
                ToolEntry entry;
                entry.tool = std::move(t);
                tools.push_back(std::move(entry));
            }
        }
    }
    if (!tools.empty()) {
        activeTool = 0;
        LoadStyleForActiveTool();
        if (tools[0].tool) tools[0].tool->OnStart();
        BuildSliders();
    }
    visible = true;
    ForceRedraw();
}

void DrawOverlay::ActivateTool(int idx) {
    if (idx < 0 || idx >= static_cast<int>(tools.size()) || idx == activeTool) return;
    if (activeTool >= 0 && tools[activeTool].tool) tools[activeTool].tool->OnEnd();
    activeTool = idx;
    LoadStyleForActiveTool();
    if (tools[activeTool].tool) tools[activeTool].tool->OnStart();
    BuildSliders();
    ForceRedraw();
}

bool DrawOverlay::ForwardEvent(const std::function<bool(pp::Tool*)>& fn) {
    if (!Loaded() || activeTool < 0 || activeTool >= static_cast<int>(tools.size())) return false;
    auto* t = tools[activeTool].tool.get();
    if (!t) return false;
    return fn(t);
}

bool DrawOverlay::ForwardToShapes(const std::function<bool(pp::Shape*)>& fn) {
    if (!canvas) return false;
    bool handled = false;
    for (auto& shp : canvas->shapes) {
        if (shp && fn(shp.get())) {
            handled = true;
        }
    }
    return handled;
}

hui::EventResult DrawOverlay::PropagateToChildren(hui::Event& event) {
    return hui::EventResult::UNHANDLED;
}

void DrawOverlay::Redraw() const {
    if (GetUI() && GetUI()->GetWindow()) {
        const dr4::Vec2f winSize = GetUI()->GetWindow()->GetSize();
        if (winSize.x > 0 && winSize.y > 0 && (winSize.x != GetSize().x || winSize.y != GetSize().y)) {
            const_cast<DrawOverlay*>(this)->SetSize(winSize);
        }
    }
    dr4::Texture& texture = GetTexture();
    texture.Clear(dr4::Color(0, 0, 0, 0));
    if (!visible) return;

    if (canvas) {
        for (auto& shp : canvas->shapes) {
            if (shp) shp->DrawOn(texture);
        }
    }
    DrawToolIcons(texture);
    DrawSettings(texture);
}

void DrawOverlay::DrawToolIcons(dr4::Texture& texture) const {
    if (!Loaded()) {

        auto* txt = GetUI()->GetWindow()->CreateText();
        txt->SetText("No PP tool plugins loaded");
        txt->SetPos(dr4::Vec2f(12, 12));
        txt->SetFontSize(13);
        txt->SetColor(dr4::Color(230, 230, 230));
        texture.Draw(*txt);
        return;
    }
    const dr4::Color panelBg(50, 50, 54, 220);
    const dr4::Color bg(60, 60, 60, 230);
    const dr4::Color hover(80, 90, 100, 235);
    const dr4::Color active(120, 170, 230, 245);

    const float pad = 8.0f;
    const float x0 = 10.0f;
    const float y0 = 46.0f;
    const float w = 38.0f;
    const float h = 38.0f;
    const float gap = 6.0f;
    const int cols = 2;

    const int rows = static_cast<int>((tools.size() + cols - 1) / cols);
    const float panelW = cols * w + (cols - 1) * gap + 2 * pad;
    const float panelH = rows * h + (rows - 1) * gap + 2 * pad;

    auto* panel = GetUI()->GetWindow()->CreateRectangle();
    panel->SetPos(dr4::Vec2f(x0 - pad, y0 - pad));
    panel->SetSize(dr4::Vec2f(panelW, panelH));
    panel->SetFillColor(panelBg);
    panel->SetBorderColor(dr4::Color(90, 90, 92, 220));
    panel->SetBorderThickness(1.0f);
    texture.Draw(*panel);

    for (size_t i = 0; i < tools.size(); ++i) {
        const int col = static_cast<int>(i) % cols;
        const int row = static_cast<int>(i) / cols;
        const float x = x0 + col * (w + gap);
        const float y = y0 + row * (h + gap);

        auto* rect = GetUI()->GetWindow()->CreateRectangle();
        rect->SetPos(dr4::Vec2f(x, y));
        rect->SetSize(dr4::Vec2f(w, h));
        if (static_cast<int>(i) == activeTool) rect->SetFillColor(active);
        else if (static_cast<int>(i) == hoveredTool) rect->SetFillColor(hover);
        else rect->SetFillColor(bg);
        rect->SetBorderColor(dr4::Color(90, 90, 90));
        rect->SetBorderThickness(1.0f);
        texture.Draw(*rect);

        
        auto* icon = GetUI()->GetWindow()->CreateImage();
        icon->SetPos(dr4::Vec2f(x + 7.0f, y + 7.0f));
        DrawToolIconBitmap(GetUI()->GetWindow(), icon, tools[i].tool ? tools[i].tool->Name() : "");
        texture.Draw(*icon);
    }
}

hui::EventResult DrawOverlay::OnMouseDown(hui::MouseButtonEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    
    if (GetUI()) GetUI()->ReportFocus(this);

    {
        const float x0 = 10.0f;
        const float y0 = 46.0f;
        const float w = 38.0f;
        const float h = 38.0f;
        const float gap = 6.0f;
        const int cols = 2;
        for (size_t i = 0; i < tools.size(); ++i) {
            const int col = static_cast<int>(i) % cols;
            const int row = static_cast<int>(i) / cols;
            dr4::Rect2f r(dr4::Vec2f(x0 + col * (w + gap), y0 + row * (h + gap)), dr4::Vec2f(w, h));
            if (r.Contains(evt.pos)) {
                ActivateTool(static_cast<int>(i));
                return hui::EventResult::HANDLED;
            }
        }
    }
    if (settingsVisible && HandleSettingsMouseDown(evt.pos)) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }

    dr4::Event::MouseButton mb{};
    mb.button = evt.button;
    mb.pos = evt.pos;
    if (ForwardEvent([&](pp::Tool* t){ return t->OnMouseDown(mb); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }

    if (ForwardToShapes([&](pp::Shape* s){ return s->OnMouseDown(mb); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

hui::EventResult DrawOverlay::OnMouseUp(hui::MouseButtonEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    if (settingsVisible) HandleSettingsMouseUp();
    dr4::Event::MouseButton mb{};
    mb.button = evt.button;
    mb.pos = evt.pos;
    if (ForwardEvent([&](pp::Tool* t){ return t->OnMouseUp(mb); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (ForwardToShapes([&](pp::Shape* s){ return s->OnMouseUp(mb); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

hui::EventResult DrawOverlay::OnMouseMove(hui::MouseMoveEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;

    {
        const float x0 = 10.0f;
        const float y0 = 46.0f;
        const float w = 38.0f;
        const float h = 38.0f;
        const float gap = 6.0f;
        const int cols = 2;
        int newHover = -1;
        for (size_t i = 0; i < tools.size(); ++i) {
            const int col = static_cast<int>(i) % cols;
            const int row = static_cast<int>(i) / cols;
            dr4::Rect2f r(dr4::Vec2f(x0 + col * (w + gap), y0 + row * (h + gap)), dr4::Vec2f(w, h));
            if (r.Contains(evt.pos)) {
                newHover = static_cast<int>(i);
                break;
            }
        }
        if (newHover != hoveredTool) {
            hoveredTool = newHover;
            ForceRedraw();
        }
    }
    dr4::Event::MouseMove mm{};
    mm.pos = evt.pos;
    mm.rel = evt.rel;
    if (settingsVisible && HandleSettingsMouseMove(evt.pos)) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (ForwardEvent([&](pp::Tool* t){ return t->OnMouseMove(mm); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (ForwardToShapes([&](pp::Shape* s){ return s->OnMouseMove(mm); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

hui::EventResult DrawOverlay::OnKeyDown(hui::KeyEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;

    if (editingSlider >= 0 && editingSlider < static_cast<int>(sliders.size())) {
        auto& s = sliders[editingSlider];
        s.caretPos = std::min(s.caretPos, s.editBuffer.size());
        bool ctrl = (evt.mods & dr4::KEYMOD_CTRL) != 0;
        if (ctrl && evt.key == dr4::KeyCode::KEYCODE_C) {
            std::string toCopy;
            if (HasSelection(s)) {
                size_t a = std::min(s.selStart, s.selEnd);
                size_t b = std::max(s.selStart, s.selEnd);
                a = std::min(a, s.editBuffer.size());
                b = std::min(b, s.editBuffer.size());
                toCopy = s.editBuffer.substr(a, b - a);
            } else {
                toCopy = s.editBuffer;
            }
            if (GetUI() && GetUI()->GetWindow()) GetUI()->GetWindow()->SetClipboard(toCopy);
            return hui::EventResult::HANDLED;
        }
        if (ctrl && evt.key == dr4::KeyCode::KEYCODE_V) {
            std::string clip;
            if (GetUI() && GetUI()->GetWindow()) clip = GetUI()->GetWindow()->GetClipboard();
            DeleteSelection(s);
            for (char ch : clip) {
                if (s.editBuffer.size() >= MaxInputLen(s)) break;
                if (ch >= '0' && ch <= '9') {
                    s.editBuffer.insert(s.editBuffer.begin() + static_cast<long>(s.caretPos), ch);
                    s.caretPos++;
                } else if (ch == '.' && !s.integer && s.editBuffer.find('.') == std::string::npos) {
                    s.editBuffer.insert(s.editBuffer.begin() + static_cast<long>(s.caretPos), ch);
                    s.caretPos++;
                } else if (ch == '-' && s.min < 0.0f && s.caretPos == 0 && s.editBuffer.find('-') == std::string::npos) {
                    s.editBuffer.insert(s.editBuffer.begin(), ch);
                    s.caretPos = 1;
                }
            }
            ClearSelection(s);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_LEFT) {
            if (s.caretPos > 0) s.caretPos--;
            ClearSelection(s);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_RIGHT) {
            if (s.caretPos < s.editBuffer.size()) s.caretPos++;
            ClearSelection(s);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_HOME) {
            s.caretPos = 0;
            ClearSelection(s);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_END) {
            s.caretPos = s.editBuffer.size();
            ClearSelection(s);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_BACKSPACE) {
            if (HasSelection(s)) DeleteSelection(s);
            else if (s.caretPos > 0 && s.caretPos <= s.editBuffer.size()) { s.editBuffer.erase(s.caretPos - 1, 1); s.caretPos--; }
            ClearSelection(s);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_DELETE) {
            if (HasSelection(s)) DeleteSelection(s);
            else if (s.caretPos < s.editBuffer.size()) s.editBuffer.erase(s.caretPos, 1);
            ClearSelection(s);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_ENTER) {
            try {
                float v = std::stof(s.editBuffer.empty() ? "0" : s.editBuffer);
                v = std::clamp(v, s.min, s.max);
                if (s.integer) v = std::round(v);
                s.value = v;
                if (s.onChange) s.onChange(s.value);
            } catch (...) {}
            s.editing = false;
            editingSlider = -1;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KeyCode::KEYCODE_ESCAPE) {
            s.editing = false;
            editingSlider = -1;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::HANDLED;
    }
    dr4::Event::KeyEvent ke{};
    ke.sym = evt.key;
    ke.mods = evt.mods;
    if (ForwardEvent([&](pp::Tool* t){ return t->OnKeyDown(ke); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

hui::EventResult DrawOverlay::OnKeyUp(hui::KeyEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    dr4::Event::KeyEvent ke{};
    ke.sym = evt.key;
    ke.mods = evt.mods;
    if (ForwardEvent([&](pp::Tool* t){ return t->OnKeyUp(ke); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

hui::EventResult DrawOverlay::OnText(hui::TextEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    if (editingSlider >= 0 && editingSlider < static_cast<int>(sliders.size())) {
        auto& s = sliders[editingSlider];
        for (const char* p = evt.text; p && *p; ++p) {
            char ch = *p;
            if (s.editBuffer.size() >= MaxInputLen(s)) break;
            if (HasSelection(s)) DeleteSelection(s);
            if (ch >= '0' && ch <= '9') {
                s.editBuffer.insert(s.editBuffer.begin() + static_cast<long>(s.caretPos), ch);
                s.caretPos++;
            } else if (ch == '.' && !s.integer && s.editBuffer.find('.') == std::string::npos) {
                s.editBuffer.insert(s.editBuffer.begin() + static_cast<long>(s.caretPos), ch);
                s.caretPos++;
            } else if (ch == '-' && s.min < 0.0f && s.caretPos == 0 && s.editBuffer.find('-') == std::string::npos) {
                s.editBuffer.insert(s.editBuffer.begin(), ch);
                s.caretPos = 1;
            }
        }
        ClearSelection(s);
        caretVisible = true;
        caretBlinkAccum = 0.0;
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    dr4::Event::TextEvent te{};
    te.unicode = evt.text;
    if (ForwardEvent([&](pp::Tool* t){ return t->OnText(te); })) {
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

hui::EventResult DrawOverlay::OnIdle(hui::IdleEvent& evt) {
    if (!visible) return hui::EventResult::UNHANDLED;
    if (editingSlider >= 0 && editingSlider < static_cast<int>(sliders.size()) && sliders[editingSlider].editing) {
        caretBlinkAccum += evt.deltaTime;
        if (caretBlinkAccum >= 0.5) {
            caretBlinkAccum = 0.0;
            caretVisible = !caretVisible;
            ForceRedraw();
        }
    }
    if (ForwardEvent([&](pp::Tool* t){ pp::IdleEvent id{evt.absTime, evt.deltaTime}; return t->OnIdle(id); })) {
        return hui::EventResult::HANDLED;
    }
    return hui::EventResult::UNHANDLED;
}

void DrawOverlay::BuildSliders() {
    sliders.clear();
    settingsVisible = false;
    editingSlider = -1;
    selectingText = false;
    selectingSlider = -1;
    if (!Loaded() || activeTool < 0 || activeTool >= static_cast<int>(tools.size()) || !tools[activeTool].tool) return;
    std::string icon = ActiveIcon();
    if (icon.empty()) return;
    StyleConfig& s = style;

    auto addStrokeRGB = [&]() {
        sliders.push_back(Slider{"Border R", 0.0f, 255.0f, static_cast<float>(s.strokeColor.r), 60.0f, 40.0f, 140.0f, false,
            [&](float v){ s.strokeColor.r = static_cast<int>(v); SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, true});
        sliders.push_back(Slider{"Border G", 0.0f, 255.0f, static_cast<float>(s.strokeColor.g), 60.0f, 62.0f, 140.0f, false,
            [&](float v){ s.strokeColor.g = static_cast<int>(v); SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, true});
        sliders.push_back(Slider{"Border B", 0.0f, 255.0f, static_cast<float>(s.strokeColor.b), 60.0f, 84.0f, 140.0f, false,
            [&](float v){ s.strokeColor.b = static_cast<int>(v); SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, true});
    };
    auto addFillRGB = [&]() {
        sliders.push_back(Slider{"Fill R", 0.0f, 255.0f, static_cast<float>(s.fillColor.r), 60.0f, 116.0f, 140.0f, false,
            [&](float v){ s.fillColor.r = static_cast<int>(v); SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, true});
        sliders.push_back(Slider{"Fill G", 0.0f, 255.0f, static_cast<float>(s.fillColor.g), 60.0f, 138.0f, 140.0f, false,
            [&](float v){ s.fillColor.g = static_cast<int>(v); SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, true});
        sliders.push_back(Slider{"Fill B", 0.0f, 255.0f, static_cast<float>(s.fillColor.b), 60.0f, 160.0f, 140.0f, false,
            [&](float v){ s.fillColor.b = static_cast<int>(v); SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, true});
        sliders.push_back(Slider{"Fill A", 0.0f, 255.0f, static_cast<float>(s.fillColor.a), 60.0f, 182.0f, 140.0f, false,
            [&](float v){ s.fillColor.a = static_cast<int>(v); SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, true});
    };
    auto addStrokeThickness = [&]() {
        sliders.push_back(Slider{"Thickness", 1.0f, 20.0f, s.strokeThickness, 60.0f, 214.0f, 140.0f, false,
            [&](float v){ s.strokeThickness = v; SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, false});
    };

    if (icon == "1" || icon == "2" || icon == "6") {
        addStrokeRGB();
        addFillRGB();
        addStrokeThickness();
        settingsVisible = true;
    } else if (icon == "3") {
        addStrokeRGB();
        addStrokeThickness();
        sliders.push_back(Slider{"Arrow Len", 5.0f, 80.0f, style.arrowHeadLength, 60.0f, 214.0f, 140.0f, false,
            [&](float v){ style.arrowHeadLength = v; SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, false});
        sliders.push_back(Slider{"Arrow Ang", 5.0f, 80.0f, style.arrowHeadAngleDeg, 60.0f, 236.0f, 140.0f, false,
            [&](float v){ style.arrowHeadAngleDeg = v; SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, false});
        settingsVisible = true;
    } else if (icon == "4") {
        addStrokeRGB();
        addStrokeThickness();
        settingsVisible = true;
    } else if (icon == "B") {
        addStrokeRGB();
        sliders.push_back(Slider{"Brush Th", 1.0f, 20.0f, style.brushThickness, 60.0f, 192.0f, 140.0f, false,
            [&](float v){ style.brushThickness = v; SaveStyleForActiveTool(); ApplyStyleToActiveTool(); }, false});
        settingsVisible = true;
    } else {
        settingsVisible = false;
    }
}

void DrawOverlay::UpdateSliderPositions() {
    float y = settingsBaseY + 30.0f;
    for (auto& s : sliders) {
        s.x = settingsBaseX + 120.0f;
        s.y = y;
        s.width = 130.0f;
        s.inputWidth = 60.0f;
        y += 28.0f;
    }
}

void DrawOverlay::DrawSettings(dr4::Texture& texture) const {
    if (!visible || !settingsVisible) return;

    float panelW = PanelSize().x;
    float panelH = PanelSize().y;
    if (GetUI() && GetUI()->GetWindow()) {
        float maxX = std::max(0.0f, GetSize().x - panelW - 10.0f);
        float maxY = std::max(0.0f, GetSize().y - panelH - 10.0f);
        auto* self = const_cast<DrawOverlay*>(this);
        self->settingsBaseX = std::clamp(settingsBaseX, 120.0f, maxX);
        self->settingsBaseY = std::clamp(settingsBaseY, 10.0f, maxY);
        self->UpdateSliderPositions();
    } else {
        const_cast<DrawOverlay*>(this)->UpdateSliderPositions();
    }

    const dr4::Color panel(80, 90, 120, 230);
    const dr4::Color textColor(245, 245, 245, 255);
    const dr4::Color track(90, 90, 110, 240);
    const dr4::Color knob(190, 210, 255, 255);
    const dr4::Color knobActive(255, 230, 160, 255);


    float headerH = 26.0f;

    auto* bg = GetUI()->GetWindow()->CreateRectangle();
    bg->SetPos(dr4::Vec2f(settingsBaseX, settingsBaseY));
    bg->SetSize(dr4::Vec2f(panelW, panelH));
    bg->SetFillColor(panel);
    bg->SetBorderColor(dr4::Color(90, 90, 90, 255));
    bg->SetBorderThickness(1.5f);
    texture.Draw(*bg);

    auto* title = GetUI()->GetWindow()->CreateText();
    title->SetText("Tool settings");
    title->SetFontSize(14);
    title->SetColor(textColor);
    title->SetPos(dr4::Vec2f(settingsBaseX + 12, settingsBaseY + 5));
    texture.Draw(*title);


    auto* closeBox = GetUI()->GetWindow()->CreateRectangle();
    closeBox->SetPos(dr4::Vec2f(settingsBaseX + panelW - 22.0f, settingsBaseY + 4.0f));
    closeBox->SetSize(dr4::Vec2f(16.0f, 16.0f));
    closeBox->SetFillColor(dr4::Color(110, 110, 130, 255));
    closeBox->SetBorderColor(dr4::Color(40, 40, 50, 255));
    closeBox->SetBorderThickness(1.0f);
    texture.Draw(*closeBox);
    auto* closeTxt = GetUI()->GetWindow()->CreateText();
    closeTxt->SetText("x");
    closeTxt->SetFontSize(12);
    closeTxt->SetColor(textColor);
    closeTxt->SetPos(dr4::Vec2f(settingsBaseX + panelW - 18.0f, settingsBaseY + 4.0f));
    texture.Draw(*closeTxt);

    for (size_t i = 0; i < sliders.size(); ++i) {
        const auto& s = sliders[i];
        auto* lbl = GetUI()->GetWindow()->CreateText();
        lbl->SetText(s.label + ":");
        lbl->SetFontSize(12);
        lbl->SetColor(textColor);
        lbl->SetPos(dr4::Vec2f(settingsBaseX + 12.0f, s.y - 2));
        texture.Draw(*lbl);

        auto* trackRect = GetUI()->GetWindow()->CreateRectangle();
        trackRect->SetPos(dr4::Vec2f(s.x, s.y + 10));
        trackRect->SetSize(dr4::Vec2f(s.width, 6));
        trackRect->SetFillColor(track);
        trackRect->SetBorderColor(dr4::Color(20, 20, 20, 200));
        trackRect->SetBorderThickness(1.0f);
        texture.Draw(*trackRect);

        float t = (s.value - s.min) / (s.max - s.min);
        float knobX = s.x + std::clamp(t, 0.0f, 1.0f) * s.width;
        auto* k = GetUI()->GetWindow()->CreateCircle();
        k->SetCenter(dr4::Vec2f(knobX, s.y + 13));
        k->SetRadius(6.0f);
        k->SetFillColor(draggingSlider == static_cast<int>(i) ? knobActive : knob);
        k->SetBorderColor(dr4::Color(10, 10, 10, 200));
        k->SetBorderThickness(1.0f);
        texture.Draw(*k);


        float boxX = s.x + s.width + 12.0f;
        float boxY = s.y - 2.0f;
        auto* box = GetUI()->GetWindow()->CreateRectangle();
        box->SetPos(dr4::Vec2f(boxX, boxY));
        box->SetSize(dr4::Vec2f(s.inputWidth, 18.0f));
        box->SetFillColor(s.editing ? dr4::Color(100, 120, 160, 255) : dr4::Color(70, 80, 110, 230));
        box->SetBorderColor(dr4::Color(30, 30, 50, 255));
        box->SetBorderThickness(1.0f);
        texture.Draw(*box);

        auto* txt = GetUI()->GetWindow()->CreateText();
        std::string valueText;
        if (s.editing) {
            valueText = s.editBuffer;
        } else {
            float displayVal = s.integer ? std::round(s.value) : std::round(s.value * 10.0f) / 10.0f;
            if (s.integer) {
                valueText = std::to_string(static_cast<int>(displayVal));
            } else {
                std::ostringstream oss;
                oss.setf(std::ios::fixed);
                oss << std::setprecision(1) << displayVal;
                valueText = oss.str();
            }
        }
        txt->SetText(valueText);
        txt->SetFontSize(12);
        txt->SetColor(textColor);
        txt->SetPos(dr4::Vec2f(boxX + 6.0f, boxY + 2.0f));
        texture.Draw(*txt);

        if (s.editing && HasSelection(s)) {
            size_t a = std::min(s.selStart, s.selEnd);
            size_t b = std::max(s.selStart, s.selEnd);
            a = std::min(a, s.editBuffer.size());
            b = std::min(b, s.editBuffer.size());
            auto* m1 = GetUI()->GetWindow()->CreateText();
            m1->SetFontSize(12);
            m1->SetText(s.editBuffer.substr(0, a));
            float w1 = m1->GetBounds().x;
            auto* m2 = GetUI()->GetWindow()->CreateText();
            m2->SetFontSize(12);
            m2->SetText(s.editBuffer.substr(0, b));
            float w2 = m2->GetBounds().x;
            float sx = boxX + 6.0f + w1;
            float ex = boxX + 6.0f + w2;
            float left = std::clamp(std::min(sx, ex), boxX + 4.0f, boxX + s.inputWidth - 4.0f);
            float right = std::clamp(std::max(sx, ex), boxX + 4.0f, boxX + s.inputWidth - 4.0f);
            auto* sel = GetUI()->GetWindow()->CreateRectangle();
            sel->SetPos(dr4::Vec2f(left, boxY + 2.0f));
            sel->SetSize(dr4::Vec2f(std::max(0.0f, right - left), 16.0f));
            sel->SetFillColor(dr4::Color(120, 170, 230, 140));
            texture.Draw(*sel);
            texture.Draw(*txt);
        }

        if (s.editing && caretVisible) {
            auto* measure = GetUI()->GetWindow()->CreateText();
            measure->SetFontSize(12);
            if (s.caretPos <= s.editBuffer.size()) {
                measure->SetText(s.editBuffer.substr(0, s.caretPos));
            } else {
                measure->SetText(s.editBuffer);
            }
            float w = measure->GetBounds().x;
            float cx = boxX + 6.0f + w;
            cx = std::clamp(cx, boxX + 4.0f, boxX + s.inputWidth - 4.0f);
            auto* caret = GetUI()->GetWindow()->CreateRectangle();
            caret->SetPos(dr4::Vec2f(cx, boxY + 3.0f));
            caret->SetSize(dr4::Vec2f(1.0f, 14.0f));
            caret->SetFillColor(textColor);
            texture.Draw(*caret);
        }
    }

    auto* strokeBox = GetUI()->GetWindow()->CreateRectangle();
    strokeBox->SetPos(dr4::Vec2f(settingsBaseX + 12, settingsBaseY + panelH - 32));
    strokeBox->SetSize(dr4::Vec2f(60, 22));
    strokeBox->SetFillColor(style.strokeColor);
    strokeBox->SetBorderColor(dr4::Color(20, 20, 20, 255));
    strokeBox->SetBorderThickness(1.0f);
    texture.Draw(*strokeBox);

    auto* fillBox = GetUI()->GetWindow()->CreateRectangle();
    fillBox->SetPos(dr4::Vec2f(settingsBaseX + 82, settingsBaseY + panelH - 32));
    fillBox->SetSize(dr4::Vec2f(60, 22));
    fillBox->SetFillColor(style.fillColor);
    fillBox->SetBorderColor(dr4::Color(20, 20, 20, 255));
    fillBox->SetBorderThickness(1.0f);
    texture.Draw(*fillBox);
}

bool DrawOverlay::HandleSettingsMouseDown(const dr4::Vec2f& pos) {
    UpdateSliderPositions();
    auto applyEditing = [&](int idx){
        if (idx < 0 || idx >= static_cast<int>(sliders.size())) return;
        auto& s = sliders[idx];
        if (!s.editing) return;

        try {
            float v = std::stof(s.editBuffer.empty() ? "0" : s.editBuffer);
            v = std::clamp(v, s.min, s.max);
            if (s.integer) v = std::round(v);
            s.value = v;
            if (s.onChange) s.onChange(s.value);
        } catch (...) {

        }
        s.editing = false;
    };
    applyEditing(editingSlider);
    editingSlider = -1;

    auto panelSize = PanelSize();
    dr4::Rect2f header(dr4::Vec2f(settingsBaseX, settingsBaseY), dr4::Vec2f(panelSize.x, 24.0f));
    dr4::Rect2f closeBtn(dr4::Vec2f(settingsBaseX + panelSize.x - 22.0f, settingsBaseY + 4.0f), dr4::Vec2f(16.0f, 16.0f));
    if (header.Contains(pos)) {
        if (closeBtn.Contains(pos)) {
            settingsVisible = false;
            return true;
        }
        draggingPanel = true;
        dragOffset = pos - dr4::Vec2f(settingsBaseX, settingsBaseY);
        return true;
    }
    for (size_t i = 0; i < sliders.size(); ++i) {
        auto& s = sliders[i];

        dr4::Rect2f box(dr4::Vec2f(s.x + s.width + 12.0f, s.y - 2.0f), dr4::Vec2f(s.inputWidth, 18.0f));
        if (box.Contains(pos)) {
            editingSlider = static_cast<int>(i);
            s.editing = true;

            float displayVal = s.integer ? std::round(s.value) : std::round(s.value * 10.0f) / 10.0f;
            if (s.integer) s.editBuffer = std::to_string(static_cast<int>(displayVal));
            else {
                std::ostringstream oss;
                oss.setf(std::ios::fixed);
                oss << std::setprecision(1) << displayVal;
                s.editBuffer = oss.str();
            }
            if (s.editBuffer.size() > MaxInputLen(s)) {
                s.editBuffer = s.editBuffer.substr(0, MaxInputLen(s));
            }
            float localX = pos.x - (s.x + s.width + 12.0f) - 6.0f;
            s.caretPos = CaretFromX(GetUI()->GetWindow(), 12.0f, s.editBuffer, localX);
            s.selStart = s.selEnd = s.caretPos;
            selectingText = true;
            selectingSlider = static_cast<int>(i);
            caretVisible = true;
            caretBlinkAccum = 0.0;
            return true;
        }
        dr4::Rect2f hit(dr4::Vec2f(s.x, s.y + 4), dr4::Vec2f(s.width, 18));
        if (hit.Contains(pos)) {
            float t = std::clamp((pos.x - s.x) / s.width, 0.0f, 1.0f);
            s.value = s.min + (s.max - s.min) * t;
            if (s.integer) s.value = std::round(s.value);
            if (s.onChange) s.onChange(s.value);
            draggingSlider = static_cast<int>(i);
            s.dragging = true;
            return true;
        }
    }
    return false;
}

bool DrawOverlay::HandleSettingsMouseMove(const dr4::Vec2f& pos) {
    if (draggingPanel) {
        auto panelSize = PanelSize();
        float panelW = panelSize.x;
        float panelH = panelSize.y;
        float newX = pos.x - dragOffset.x;
        float newY = pos.y - dragOffset.y;
        float maxX = std::max(0.0f, GetSize().x - panelW - 10.0f);
        float maxY = std::max(0.0f, GetSize().y - panelH - 10.0f);
        settingsBaseX = std::clamp(newX, 10.0f, maxX);
        settingsBaseY = std::clamp(newY, 10.0f, maxY);
        UpdateSliderPositions();
        return true;
    }
    if (selectingText && selectingSlider >= 0 && selectingSlider < static_cast<int>(sliders.size())) {
        auto& s = sliders[selectingSlider];
        float localX = pos.x - (s.x + s.width + 12.0f) - 6.0f;
        size_t cp = CaretFromX(GetUI()->GetWindow(), 12.0f, s.editBuffer, localX);
        s.caretPos = cp;
        s.selEnd = cp;
        caretVisible = true;
        caretBlinkAccum = 0.0;
        return true;
    }
    if (draggingSlider < 0 || draggingSlider >= static_cast<int>(sliders.size())) return false;
    auto& s = sliders[draggingSlider];
    if (s.editing) return false;
    float t = std::clamp((pos.x - s.x) / s.width, 0.0f, 1.0f);
    s.value = s.min + (s.max - s.min) * t;
    if (s.integer) s.value = std::round(s.value);
    if (s.onChange) s.onChange(s.value);
    return true;
}

void DrawOverlay::HandleSettingsMouseUp() {
    if (draggingSlider >= 0 && draggingSlider < static_cast<int>(sliders.size())) {
        sliders[draggingSlider].dragging = false;
    }
    draggingSlider = -1;
    draggingPanel = false;
    selectingText = false;
    selectingSlider = -1;
}

std::string DrawOverlay::ActiveIcon() const {
    if (!Loaded() || activeTool < 0 || activeTool >= static_cast<int>(tools.size()) || !tools[activeTool].tool) return {};
    std::string icon = std::string(tools[activeTool].tool->Icon());
    return icon;
}

void DrawOverlay::LoadStyleForActiveTool() {
    std::string key = ActiveIcon();
    if (key.empty()) {
        style = defaultStyle;
        return;
    }
    auto it = stylePerTool.find(key);
    if (it != stylePerTool.end()) {
        style = it->second;
    } else {
        style = defaultStyle;
    }
}

void DrawOverlay::SaveStyleForActiveTool() {
    std::string key = ActiveIcon();
    if (key.empty()) return;
    stylePerTool[key] = style;
}

void DrawOverlay::ApplyStyleToActiveTool() {
    if (!Loaded() || activeTool < 0 || activeTool >= static_cast<int>(tools.size())) return;
    if (!canvas) return;
    pp::Shape* shp = canvas->GetSelectedShape();
    if (!shp) return;
    std::string icon = ActiveIcon();
    if (icon == "1") {
        auto* r = static_cast<pp::Rectangle*>(shp);
        r->SetBorderColor(style.strokeColor);
        r->SetBorderThickness(style.strokeThickness);
        r->SetFillColor(style.fillColor);
    } else if (icon == "2") {
        auto* c = static_cast<pp::Circle*>(shp);
        c->SetBorderColor(style.strokeColor);
        c->SetBorderThickness(style.strokeThickness);
        c->SetFillColor(style.fillColor);
    } else if (icon == "3") {
        auto* a = static_cast<pp::Arrow*>(shp);
        a->SetStrokeColor(style.strokeColor);
        a->SetWidth(style.strokeThickness);
        a->SetArrowLen(style.arrowHeadLength);
        a->SetArrowAngleDeg(style.arrowHeadAngleDeg);
    } else if (icon == "4") {
        auto* l = static_cast<pp::Line*>(shp);
        l->SetBorderColor(style.strokeColor);
        l->SetBorderThickness(style.strokeThickness);
    } else if (icon == "6") {
        auto* h = static_cast<pp::Heart*>(shp);
        h->SetStrokeColor(style.strokeColor);
        h->SetBorderThickness(style.strokeThickness);
        h->SetFillColor(style.fillColor);
    } else if (icon == "B") {
        auto* b = static_cast<pp::BrushStroke*>(shp);
        b->SetColor(style.strokeColor);
        b->SetThickness(style.brushThickness);
    }
}

dr4::Vec2f DrawOverlay::PanelSize() const {
    float panelW = 360.0f;
    float panelH = 30.0f + static_cast<float>(sliders.size()) * 28.0f + 48.0f;
    return dr4::Vec2f(panelW, panelH);
}

size_t DrawOverlay::MaxInputLen(const Slider& s) {
    float maxAbs = std::max(std::fabs(s.min), std::fabs(s.max));
    int maxInt = static_cast<int>(std::ceil(maxAbs));
    size_t digits = std::to_string(maxInt).size();
    size_t len = digits;
    if (s.min < 0.0f) len += 1;
    if (!s.integer) len += 2;
    return len;
}

bool DrawOverlay::HasSelection(const Slider& s) {
    return s.selStart != s.selEnd;
}

void DrawOverlay::ClearSelection(Slider& s) {
    s.selStart = s.selEnd = s.caretPos;
}

void DrawOverlay::DeleteSelection(Slider& s) {
    if (!HasSelection(s)) return;
    size_t a = std::min(s.selStart, s.selEnd);
    size_t b = std::max(s.selStart, s.selEnd);
    a = std::min(a, s.editBuffer.size());
    b = std::min(b, s.editBuffer.size());
    s.editBuffer.erase(a, b - a);
    s.caretPos = a;
    ClearSelection(s);
}

size_t DrawOverlay::CaretFromX(dr4::Window* w, float fontSize, const std::string& txt, float localX) {
    if (!w) return 0;
    if (localX <= 0.0f) return 0;
    auto* t = w->CreateText();
    t->SetFontSize(fontSize);
    size_t best = 0;
    for (size_t i = 0; i <= txt.size(); ++i) {
        t->SetText(txt.substr(0, i));
        float x = t->GetBounds().x;
        if (x <= localX + 0.5f) best = i;
        else break;
    }
    return best;
}

} // namespace ui

