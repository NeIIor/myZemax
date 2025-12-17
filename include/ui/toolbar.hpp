#ifndef UI_TOOLBAR_HPP
#define UI_TOOLBAR_HPP

#include "hui/container.hpp"
#include "cum/manager.hpp"
#include <functional>
#include <string>
#include <vector>

namespace hui {
    class MouseButtonEvent;
}

namespace ui {

class Toolbar : public hui::Container {
public:
    Toolbar(hui::UI* ui, cum::Manager* manager);
    
    void SetupMenu();
    void SetManager(cum::Manager* manager);
    void SetDrawMode(bool v) { drawMode = v; ForceRedraw(); }
    void SetPluginsDirectory(std::string dir) { pluginsDir = std::move(dir); ForceRedraw(); }
    void SetActiveToolsPluginLabel(std::string lbl) { activeToolsPluginLabel = std::move(lbl); ForceRedraw(); }
    void SetOnToolsPluginSelected(std::function<void(const std::string& soPath)> cb) { onToolsPluginSelected = std::move(cb); }

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;
    hui::EventResult OnMouseMove(hui::MouseMoveEvent& evt) override;
    hui::EventResult OnMouseWheel(hui::MouseWheelEvent& evt) override;

private:
    cum::Manager* pluginManager;
    bool pluginsMenuOpen = false;
    bool drawMode = false;
    
    void DrawPluginsMenu(dr4::Texture& texture) const;

    enum class MenuPage : uint8_t { Root = 0, Tools, Backend };
    MenuPage menuPage = MenuPage::Root;
    std::string pluginsDir;
    std::string activeToolsPluginLabel = "none";
    std::function<void(const std::string& soPath)> onToolsPluginSelected;

    
    int hoverIndex = -1;
    float scrollOffset = 0.0f;
    mutable std::vector<std::string> cachedToolsSos;
    mutable std::vector<std::string> cachedBackendSos;

    void RefreshCaches() const;
};

} // namespace ui

#endif // UI_TOOLBAR_HPP