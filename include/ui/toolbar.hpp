#ifndef UI_TOOLBAR_HPP
#define UI_TOOLBAR_HPP

#include "hui/container.hpp"
#include "cum/manager.hpp"

namespace ui {

class Toolbar : public hui::Container {
public:
    Toolbar(hui::UI* ui, cum::Manager* manager);
    
    void SetupMenu();
    void SetManager(cum::Manager* manager);

protected:
    void Redraw() const override;
    EventResult OnMouseDown(MouseButtonEvent& evt) override;

private:
    cum::Manager* pluginManager;
    bool pluginsMenuOpen = false;
    
    void DrawPluginsMenu(dr4::Texture& texture) const;
};

} // namespace ui

#endif // UI_TOOLBAR_HPP

