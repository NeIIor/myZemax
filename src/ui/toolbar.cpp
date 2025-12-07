#include "ui/toolbar.hpp"
#include "cum/plugin.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"  // Добавляю для GetUI()->GetWindow()

namespace ui {

Toolbar::Toolbar(hui::UI* ui, cum::Manager* manager)
    : Container(ui), pluginManager(manager) {
    SetSize(1280, 30);
}

hui::EventResult Toolbar::PropagateToChildren(hui::Event& event) {
    return hui::EventResult::UNHANDLED;
}

void Toolbar::SetManager(cum::Manager* manager) {
    pluginManager = manager;
    ForceRedraw();
}

void Toolbar::SetupMenu() {
}

void Toolbar::Redraw() const {
    dr4::Texture& texture = GetTexture();
    texture.Clear(dr4::Color(40, 40, 40));
    
    auto* text = GetUI()->GetWindow()->CreateText();
    text->SetText("Plugins");
    text->SetPos(dr4::Vec2f(10, 5));
    text->SetFontSize(14);
    text->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*text);
    
    if (pluginsMenuOpen && pluginManager) {
        DrawPluginsMenu(texture);
    }
}

void Toolbar::DrawPluginsMenu(dr4::Texture& texture) const {
    auto* rect = GetUI()->GetWindow()->CreateRectangle();
    rect->SetPos(dr4::Vec2f(10, 30));
    rect->SetSize(dr4::Vec2f(300, 300));
    rect->SetFillColor(dr4::Color(60, 60, 60));
    rect->SetBorderColor(dr4::Color(100, 100, 100));
    rect->SetBorderThickness(1.0f);
    texture.Draw(*rect);
    
    if (!pluginManager) {
        auto* text = GetUI()->GetWindow()->CreateText();
        text->SetText("Plugin manager not initialized");
        text->SetPos(dr4::Vec2f(20, 40));
        text->SetFontSize(12);
        text->SetColor(dr4::Color(200, 200, 200));
        texture.Draw(*text);
        return;
    }
    
    const auto& plugins = pluginManager->GetAll();
    if (plugins.empty()) {
        auto* text = GetUI()->GetWindow()->CreateText();
        text->SetText("No plugins loaded");
        text->SetPos(dr4::Vec2f(20, 40));
        text->SetFontSize(12);
        text->SetColor(dr4::Color(200, 200, 200));
        texture.Draw(*text);
        return;
    }
    
    float y = 40.0f;
    float lineHeight = 25.0f;
    
    for (const auto& plugin : plugins) {
        auto* text = GetUI()->GetWindow()->CreateText();
        std::string name = std::string(plugin->GetName());
        text->SetText(name);
        text->SetPos(dr4::Vec2f(20, y));
        text->SetFontSize(12);
        text->SetColor(dr4::Color(255, 255, 255));
        texture.Draw(*text);
        
        y += lineHeight;
        std::string id = "ID: " + std::string(plugin->GetIdentifier());
        text->SetText(id);
        text->SetPos(dr4::Vec2f(30, y));
        text->SetFontSize(10);
        text->SetColor(dr4::Color(180, 180, 180));
        texture.Draw(*text);
        
        y += lineHeight + 5.0f;
        
        if (y > 320.0f) break;
    }
}

hui::EventResult Toolbar::OnMouseDown(hui::MouseButtonEvent& evt) {
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (evt.pos.x >= 10 && evt.pos.x <= 100 && evt.pos.y >= 0 && evt.pos.y <= 30) {
            pluginsMenuOpen = !pluginsMenuOpen;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        
        if (pluginsMenuOpen && (evt.pos.x < 10 || evt.pos.x > 310 || evt.pos.y < 30 || evt.pos.y > 330)) {
            pluginsMenuOpen = false;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
    }
    
    return Container::OnMouseDown(evt);
}

} // namespace ui