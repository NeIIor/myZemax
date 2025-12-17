#include "ui/toolbar.hpp"
#include "cum/plugin.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"
#include <filesystem>
#include <algorithm>

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
    const dr4::Color bg(60, 60, 60);
    const dr4::Color textColor(235, 235, 235);
    const dr4::Color accent(120, 170, 230);
    
    texture.Clear(dr4::Color(0, 0, 0, 0));

    const float barH = 36.0f;
    auto* bar = GetUI()->GetWindow()->CreateRectangle();
    bar->SetPos(dr4::Vec2f(0, 0));
    bar->SetSize(dr4::Vec2f(GetSize().x, barH));
    bar->SetFillColor(bg);
    texture.Draw(*bar);
    
    auto* text = GetUI()->GetWindow()->CreateText();
    std::string title = "Plugins";
    title += " (tools: " + activeToolsPluginLabel + ")";
    if (drawMode) title += " (Draw mode: ON)";
    text->SetText(title);
    text->SetPos(dr4::Vec2f(10, 8));
    text->SetFontSize(16);
    text->SetColor(drawMode ? accent : textColor);
    texture.Draw(*text);
    
    if (pluginsMenuOpen && pluginManager) {
        DrawPluginsMenu(texture);
    }
}

void Toolbar::RefreshCaches() const {
    cachedToolsSos.clear();
    cachedBackendSos.clear();
    if (pluginsDir.empty()) return;

    namespace fs = std::filesystem;
    const fs::path tools = fs::path(pluginsDir) / "tools";
    const fs::path backend = fs::path(pluginsDir) / "backend";

    auto collect = [&](const fs::path& dir, std::vector<std::string>& out) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) return;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;
            auto p = entry.path();
            if (p.extension() == ".so") {
                out.push_back(p.filename().string());
            }
        }
        std::sort(out.begin(), out.end());
    };

    collect(tools, cachedToolsSos);
    collect(backend, cachedBackendSos);
}

void Toolbar::DrawPluginsMenu(dr4::Texture& texture) const {
    const dr4::Color border(100, 100, 100);
    const dr4::Color bg(60, 60, 60);
    const dr4::Color itemHover(88, 98, 110);
    const dr4::Color textMain(235, 235, 235);
    const dr4::Color textSub(180, 180, 180);

    const float mx = 10.0f;
    const float my = 36.0f;
    const float mw = 320.0f;
    const float mh = 220.0f;

    auto* rect = GetUI()->GetWindow()->CreateRectangle();
    rect->SetPos(dr4::Vec2f(mx, my));
    rect->SetSize(dr4::Vec2f(mw, mh));
    rect->SetFillColor(bg);
    rect->SetBorderColor(border);
    rect->SetBorderThickness(1.0f);
    texture.Draw(*rect);
    
    if (!pluginManager) {
        auto* text = GetUI()->GetWindow()->CreateText();
        text->SetText("Plugin manager not initialized");
        text->SetPos(dr4::Vec2f(mx + 10, my + 10));
        text->SetFontSize(12);
        text->SetColor(dr4::Color(200, 200, 200));
        texture.Draw(*text);
        return;
    }

    RefreshCaches();

    
    auto* header = GetUI()->GetWindow()->CreateText();
    header->SetFontSize(12);
    header->SetColor(textSub);
    header->SetPos(dr4::Vec2f(mx + 10, my + 8));
    if (menuPage == MenuPage::Root) header->SetText("Select: backend / tools");
    if (menuPage == MenuPage::Tools) header->SetText("tools / (click to load)");
    if (menuPage == MenuPage::Backend) header->SetText("backend / (not switchable yet)");
    texture.Draw(*header);

    float listX = mx + 10.0f;
    float listY = my + 30.0f;
    float listW = mw - 20.0f;
    float listH = mh - 40.0f;
    const float itemH = 22.0f;

    std::vector<std::string> items;
    if (menuPage == MenuPage::Root) {
        items.push_back("backend");
        items.push_back("tools");
    } else if (menuPage == MenuPage::Tools) {
        items = cachedToolsSos;
    } else if (menuPage == MenuPage::Backend) {
        items = cachedBackendSos;
    }

    float contentH = static_cast<float>(items.size()) * itemH;
    float maxScroll = std::max(0.0f, contentH - listH);
    float y = listY - scrollOffset;

    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        float iy = y + i * itemH;
        if (iy + itemH < listY || iy > listY + listH) continue;
        dr4::Rect2f itemRect(dr4::Vec2f(listX, iy), dr4::Vec2f(listW, itemH));
        if (i == hoverIndex) {
            auto* r = GetUI()->GetWindow()->CreateRectangle();
            r->SetPos(itemRect.pos);
            r->SetSize(itemRect.size);
            r->SetFillColor(itemHover);
            texture.Draw(*r);
        }
        auto* t = GetUI()->GetWindow()->CreateText();
        t->SetText(items[i]);
        t->SetPos(dr4::Vec2f(itemRect.pos.x + 6, itemRect.pos.y + 4));
        t->SetFontSize(12);
        t->SetColor(textMain);
        texture.Draw(*t);
    }

    if (maxScroll > 0.0f) {
        
        float trackX = mx + mw - 12.0f;
        float trackY = listY;
        float trackH = listH;
        auto* track = GetUI()->GetWindow()->CreateRectangle();
        track->SetPos(dr4::Vec2f(trackX, trackY));
        track->SetSize(dr4::Vec2f(8.0f, trackH));
        track->SetFillColor(dr4::Color(50, 50, 50));
        texture.Draw(*track);

        float thumbH = std::max(18.0f, trackH * (listH / contentH));
        float t = (maxScroll <= 0.0f) ? 0.0f : (scrollOffset / maxScroll);
        float thumbY = trackY + t * (trackH - thumbH);
        auto* thumb = GetUI()->GetWindow()->CreateRectangle();
        thumb->SetPos(dr4::Vec2f(trackX + 1.0f, thumbY));
        thumb->SetSize(dr4::Vec2f(6.0f, thumbH));
        thumb->SetFillColor(dr4::Color(110, 110, 116));
        texture.Draw(*thumb);
    }
}

hui::EventResult Toolbar::OnMouseDown(hui::MouseButtonEvent& evt) {
    if (evt.button == dr4::MouseButtonType::LEFT) {
        const float barH = 36.0f;
        const bool inBar = (evt.pos.y >= 0 && evt.pos.y <= barH);

        if (evt.pos.x >= 10 && evt.pos.x <= 320 && inBar) {
            pluginsMenuOpen = !pluginsMenuOpen;
            menuPage = MenuPage::Root;
            hoverIndex = -1;
            scrollOffset = 0.0f;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        
        if (pluginsMenuOpen) {
            const float mx = 10.0f;
            const float my = barH;
            const float mw = 320.0f;
            const float mh = 220.0f;
            const bool inMenu = (evt.pos.x >= mx && evt.pos.x <= mx + mw && evt.pos.y >= my && evt.pos.y <= my + mh);
            if (!inMenu) {
                
                pluginsMenuOpen = false;
                hoverIndex = -1;
                scrollOffset = 0.0f;
                ForceRedraw();
                return hui::EventResult::UNHANDLED;
            }

            RefreshCaches();
            
            if (evt.pos.x >= mx + 10.0f && evt.pos.x <= mx + mw - 10.0f && evt.pos.y >= my && evt.pos.y <= my + 24.0f) {
                if (menuPage != MenuPage::Root) {
                    menuPage = MenuPage::Root;
                    hoverIndex = -1;
                    scrollOffset = 0.0f;
                    ForceRedraw();
                    return hui::EventResult::HANDLED;
                }
            }
            float listX = mx + 10.0f;
            float listY = my + 30.0f;
            float listW = mw - 20.0f;
            float listH = mh - 40.0f;
            const float itemH = 22.0f;
            dr4::Rect2f listRect(dr4::Vec2f(listX, listY), dr4::Vec2f(listW, listH));
            if (listRect.Contains(evt.pos)) {
                std::vector<std::string> items;
                if (menuPage == MenuPage::Root) {
                    items.push_back("backend");
                    items.push_back("tools");
                } else if (menuPage == MenuPage::Tools) {
                    items = cachedToolsSos;
                } else if (menuPage == MenuPage::Backend) {
                    items = cachedBackendSos;
                }

                int idx = static_cast<int>(((evt.pos.y - listY) + scrollOffset) / itemH);
                if (idx >= 0 && idx < static_cast<int>(items.size())) {
                    if (menuPage == MenuPage::Root) {
                        if (items[idx] == "tools") {
                            menuPage = MenuPage::Tools;
                            hoverIndex = -1;
                            scrollOffset = 0.0f;
                            ForceRedraw();
                            return hui::EventResult::HANDLED;
                        }
                        if (items[idx] == "backend") {
                            menuPage = MenuPage::Backend;
                            hoverIndex = -1;
                            scrollOffset = 0.0f;
                            ForceRedraw();
                            return hui::EventResult::HANDLED;
                        }
                    } else if (menuPage == MenuPage::Tools) {
                        if (!pluginsDir.empty() && onToolsPluginSelected) {
                            std::string full = (std::filesystem::path(pluginsDir) / "tools" / items[idx]).string();
                            onToolsPluginSelected(full);
                        }
                        pluginsMenuOpen = false;
                        menuPage = MenuPage::Root;
                        hoverIndex = -1;
                        scrollOffset = 0.0f;
                        ForceRedraw();
                        return hui::EventResult::HANDLED;
                    } else {
                        
                        ForceRedraw();
                        return hui::EventResult::HANDLED;
                    }
                }
            }
            return hui::EventResult::HANDLED;
        }
    }
    
    
    return hui::EventResult::UNHANDLED;
}

hui::EventResult Toolbar::OnMouseMove(hui::MouseMoveEvent& evt) {
    const float barH = 36.0f;
    const bool inBar = (evt.pos.y >= 0 && evt.pos.y <= barH);
    const float mx = 10.0f;
    const float my = barH;
    const float mw = 320.0f;
    const float mh = 220.0f;
    const bool inMenu = pluginsMenuOpen && (evt.pos.x >= mx && evt.pos.x <= mx + mw && evt.pos.y >= my && evt.pos.y <= my + mh);

    if (!pluginsMenuOpen && !inBar) {
        return hui::EventResult::UNHANDLED;
    }

    if (!pluginsMenuOpen && inBar) {
        
        return hui::EventResult::UNHANDLED;
    }
    const float listX = mx + 10.0f;
    const float listY = my + 30.0f;
    const float listW = mw - 20.0f;
    const float listH = mh - 40.0f;
    const float itemH = 22.0f;

    RefreshCaches();
    std::vector<std::string> items;
    if (menuPage == MenuPage::Root) items = {"backend", "tools"};
    else if (menuPage == MenuPage::Tools) items = cachedToolsSos;
    else items = cachedBackendSos;

    dr4::Rect2f listRect(dr4::Vec2f(listX, listY), dr4::Vec2f(listW, listH));
    int newHover = -1;
    if (listRect.Contains(evt.pos)) {
        int idx = static_cast<int>(((evt.pos.y - listY) + scrollOffset) / itemH);
        if (idx >= 0 && idx < static_cast<int>(items.size())) newHover = idx;
    }
    if (newHover != hoverIndex) {
        hoverIndex = newHover;
        ForceRedraw();
    }
    return inMenu ? hui::EventResult::HANDLED : hui::EventResult::UNHANDLED;
}

hui::EventResult Toolbar::OnMouseWheel(hui::MouseWheelEvent& evt) {
    if (!pluginsMenuOpen) return hui::EventResult::UNHANDLED;
    if (menuPage == MenuPage::Root) return hui::EventResult::HANDLED;
    RefreshCaches();

    const float mh = 220.0f;
    const float listH = mh - 40.0f;
    const float itemH = 22.0f;
    size_t n = (menuPage == MenuPage::Tools) ? cachedToolsSos.size() : cachedBackendSos.size();
    float contentH = static_cast<float>(n) * itemH;
    float maxScroll = std::max(0.0f, contentH - listH);
    scrollOffset -= evt.delta.y * 20.0f;
    scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);
    ForceRedraw();
    return hui::EventResult::HANDLED;
}

} // namespace ui