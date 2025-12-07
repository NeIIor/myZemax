#ifndef UI_TOOLBAR_HPP
#define UI_TOOLBAR_HPP

#include "hui/container.hpp"
#include "cum/manager.hpp"

namespace hui {
    class MouseButtonEvent;
}

namespace ui {

class Toolbar : public hui::Container {
public:
    Toolbar(hui::UI* ui, cum::Manager* manager);
    
    void SetupMenu();
    void SetManager(cum::Manager* manager);

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override;
    void Redraw() const override;
    hui::EventResult OnMouseDown(hui::MouseButtonEvent& evt) override;

private:
    cum::Manager* pluginManager;
    bool pluginsMenuOpen = false;
    
    void DrawPluginsMenu(dr4::Texture& texture) const;
};

} // namespace ui

#endif // UI_TOOLBAR_HPP