#ifndef UI_DRAW_OVERLAY_HPP
#define UI_DRAW_OVERLAY_HPP

#include "hui/container.hpp"
#include "cum/ifc/pp.hpp"
#include "cum/manager.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

namespace ui {

class DrawOverlay : public hui::Container {
public:
    struct StyleConfig {
        dr4::Color strokeColor = dr4::Color(200, 200, 220, 255);
        dr4::Color fillColor = dr4::Color(80, 120, 160, 120);
        float strokeThickness = 4.0f;
        float arrowHeadLength = 22.0f;
        float arrowHeadAngleDeg = 28.0f;
        float brushThickness = 4.0f;
    };

    DrawOverlay(hui::UI* ui);
    void SetPluginManager(cum::Manager* mgr) { pluginManager = mgr; }
    
    
    void SetToolsPluginIdFilter(std::string id);
    void EnsureLoaded();
    void ToggleVisible(bool v) { visible = v; ForceRedraw(); }

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnMouseUp(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent& evt) override;
    hui::EventResult OnKeyDown(hui::KeyEvent& evt) override;
    hui::EventResult OnKeyUp(hui::KeyEvent& evt) override;
    hui::EventResult OnText(hui::TextEvent& evt) override;
public:
    hui::EventResult OnIdle(hui::IdleEvent& evt) override;

private:
    cum::Manager* pluginManager = nullptr;
    std::string toolsPluginIdFilter;
    struct CanvasImpl;
    std::unique_ptr<CanvasImpl> canvas;

    struct ToolEntry {
        std::unique_ptr<pp::Tool> tool;
    };
    std::vector<ToolEntry> tools;
    int activeTool = -1;
    bool visible = false;
    bool draggingShape = false;

    bool Loaded() const { return !tools.empty(); }
    void ActivateTool(int idx);
    bool ForwardEvent(const std::function<bool(pp::Tool*)>& fn);
    bool ForwardToShapes(const std::function<bool(pp::Shape*)>& fn);
    void DrawToolIcons(dr4::Texture& texture) const;
    void DrawSettings(dr4::Texture& texture) const;
    bool HandleSettingsMouseDown(const dr4::Vec2f& pos);
    bool HandleSettingsMouseMove(const dr4::Vec2f& pos);
    void HandleSettingsMouseUp();
    dr4::Vec2f PanelSize() const;

    struct Slider {
        std::string label;
        float min = 0.0f;
        float max = 1.0f;
        float value = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float width = 120.0f;
        bool dragging = false;
        std::function<void(float)> onChange;
        bool integer = false;

        float inputWidth = 60.0f;
        bool editing = false;
        std::string editBuffer;
        size_t caretPos = 0;
        size_t selStart = 0;
        size_t selEnd = 0;
    };

    void BuildSliders();
    void UpdateSliderPositions();
    static size_t MaxInputLen(const Slider& s);
    static bool HasSelection(const Slider& s);
    static void ClearSelection(Slider& s);
    static void DeleteSelection(Slider& s);
    static size_t CaretFromX(dr4::Window* w, float fontSize, const std::string& txt, float localX);
    std::vector<Slider> sliders;
    int draggingSlider = -1;
    int editingSlider = -1;
    bool selectingText = false;
    int selectingSlider = -1;
    bool caretVisible = true;
    double caretBlinkAccum = 0.0;
    StyleConfig style;
    StyleConfig defaultStyle{};
    std::unordered_map<std::string, StyleConfig> stylePerTool;
    float settingsBaseX = 70.0f;
    float settingsBaseY = 20.0f;
    bool settingsVisible = false;
    bool draggingPanel = false;
    dr4::Vec2f dragOffset{};
    int hoveredTool = -1;

    std::string ActiveIcon() const;
    void LoadStyleForActiveTool();
    void SaveStyleForActiveTool();
    void ApplyStyleToActiveTool();

    void ResetTools();
};

} // namespace ui

#endif

