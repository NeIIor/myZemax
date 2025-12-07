#include "ui/properties_window.hpp"
#include "raytracer/object.hpp"
#include "raytracer/objects.hpp"
#include "raytracer/scene.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "dr4/keycodes.hpp"

namespace ui {

PropertiesWindow::PropertiesWindow(hui::UI* ui) : Container(ui) {
    SetSize(300, 400);
}

void PropertiesWindow::SetObject(raytracer::Object* obj) {
    currentObject = obj;
    UpdateFromObject();
    ForceRedraw();
}

void PropertiesWindow::UpdateFromObject() {
    if (!currentObject) {
        nameText = "";
        posXText = posYText = posZText = "";
        colorRText = colorGText = colorBText = "";
        refractiveIndexText = "";
        reflectivityText = "";
        return;
    }
    
    nameText = currentObject->name;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << currentObject->position.x;
    posXText = oss.str();
    oss.str("");
    oss << currentObject->position.y;
    posYText = oss.str();
    oss.str("");
    oss << currentObject->position.z;
    posZText = oss.str();
    oss.str("");
    
    oss << static_cast<int>(currentObject->color.r);
    colorRText = oss.str();
    oss.str("");
    oss << static_cast<int>(currentObject->color.g);
    colorGText = oss.str();
    oss.str("");
    oss << static_cast<int>(currentObject->color.b);
    colorBText = oss.str();
    oss.str("");
    
    oss << currentObject->refractiveIndex;
    refractiveIndexText = oss.str();
    oss.str("");
    oss << currentObject->reflectivity;
    reflectivityText = oss.str();
}

void PropertiesWindow::Redraw() const {
    dr4::Texture& texture = GetTexture();
    texture.Clear(dr4::Color(60, 60, 60));
    
    auto* titleText = GetWindow()->CreateText();
    titleText->SetText("Properties");
    titleText->SetPos(dr4::Vec2f(10, 5));
    titleText->SetFontSize(14);
    titleText->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*titleText);
    
    auto* arrowText = GetWindow()->CreateText();
    arrowText->SetText(isCollapsed ? "▼" : "▶");
    arrowText->SetPos(dr4::Vec2f(GetSize().x - 20, 5));
    arrowText->SetFontSize(14);
    arrowText->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*arrowText);
    
    if (isCollapsed) return;
    
    if (!currentObject) {
        auto* text = GetWindow()->CreateText();
        text->SetText("No object selected");
        text->SetPos(dr4::Vec2f(10, 10));
        text->SetFontSize(14);
        text->SetColor(dr4::Color(200, 200, 200));
        texture.Draw(*text);
        return;
    }
    
    float y = 10.0f;
    float lineHeight = 25.0f;
    
    auto* label = GetWindow()->CreateText();
    label->SetText("Name:");
    label->SetPos(dr4::Vec2f(10, y));
    label->SetFontSize(12);
    label->SetColor(dr4::Color(255, 255, 255));
    texture.Draw(*label);
    
    auto* value = GetWindow()->CreateText();
    value->SetText(nameText);
    value->SetPos(dr4::Vec2f(80, y));
    value->SetFontSize(12);
    value->SetColor(activeField == 0 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    
    label->SetText("Position X:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(posXText);
    value->SetPos(dr4::Vec2f(80, y));
    value->SetColor(activeField == 1 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    label->SetText("Position Y:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(posYText);
    value->SetPos(dr4::Vec2f(80, y));
    value->SetColor(activeField == 2 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    label->SetText("Position Z:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(posZText);
    value->SetPos(dr4::Vec2f(80, y));
    value->SetColor(activeField == 3 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    
    label->SetText("Color R:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(colorRText);
    value->SetPos(dr4::Vec2f(80, y));
    value->SetColor(activeField == 4 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    label->SetText("Color G:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(colorGText);
    value->SetPos(dr4::Vec2f(80, y));
    value->SetColor(activeField == 5 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    label->SetText("Color B:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(colorBText);
    value->SetPos(dr4::Vec2f(80, y));
    value->SetColor(activeField == 6 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    
    label->SetText("Refractive Index:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(refractiveIndexText);
    value->SetPos(dr4::Vec2f(120, y));
    value->SetColor(activeField == 7 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
    
    y += lineHeight;
    
    label->SetText("Reflectivity:");
    label->SetPos(dr4::Vec2f(10, y));
    texture.Draw(*label);
    value->SetText(reflectivityText);
    value->SetPos(dr4::Vec2f(120, y));
    value->SetColor(activeField == 8 ? dr4::Color(255, 255, 0) : dr4::Color(200, 200, 200));
    texture.Draw(*value);
}

EventResult PropertiesWindow::OnText(TextEvent& evt) {
    if (activeField >= 0 && currentObject) {
        ParseAndApplyChanges();
    }
    return Container::OnText(evt);
}

void PropertiesWindow::ParseAndApplyChanges() {
    if (!currentObject) return;
    
    try {
        if (activeField == 0) {
            currentObject->name = nameText;
        } else if (activeField == 1) {
            currentObject->position.x = std::stof(posXText);
        } else if (activeField == 2) {
            currentObject->position.y = std::stof(posYText);
        } else if (activeField == 3) {
            currentObject->position.z = std::stof(posZText);
        } else if (activeField == 4) {
            int r = std::stoi(colorRText);
            currentObject->color.r = static_cast<uint8_t>(std::max(0, std::min(255, r)));
        } else if (activeField == 5) {
            int g = std::stoi(colorGText);
            currentObject->color.g = static_cast<uint8_t>(std::max(0, std::min(255, g)));
        } else if (activeField == 6) {
            int b = std::stoi(colorBText);
            currentObject->color.b = static_cast<uint8_t>(std::max(0, std::min(255, b)));
        } else if (activeField == 7) {
            currentObject->refractiveIndex = std::stof(refractiveIndexText);
        } else if (activeField == 8) {
            currentObject->reflectivity = std::stof(reflectivityText);
            currentObject->reflectivity = std::max(0.0f, std::min(1.0f, currentObject->reflectivity));
        }
    } catch (...) {
    }
    
    ForceRedraw();
}

void PropertiesWindow::CopyObject() {
    if (currentObject) {
        copiedObject = CloneObject(currentObject);
    }
}

void PropertiesWindow::PasteObject(raytracer::Scene* scene) {
    if (!copiedObject || !scene) return;
    
    raytracer::Object* newObj = CloneObject(copiedObject);
    newObj->name += " (Copy)";
    newObj->position += raytracer::Vec3(1.0f, 0.0f, 0.0f);
    
    scene->AddObject(std::unique_ptr<raytracer::Object>(newObj));
    SetObject(newObj);
}

raytracer::Object* PropertiesWindow::CloneObject(raytracer::Object* obj) const {
    if (!obj) return nullptr;
    
    if (auto* sphere = dynamic_cast<raytracer::Sphere*>(obj)) {
        auto* newSphere = new raytracer::Sphere(sphere->radius, sphere->name);
        newSphere->position = sphere->position;
        newSphere->color = sphere->color;
        newSphere->refractiveIndex = sphere->refractiveIndex;
        newSphere->reflectivity = sphere->reflectivity;
        newSphere->isLightSource = sphere->isLightSource;
        return newSphere;
    } else if (auto* plane = dynamic_cast<raytracer::Plane*>(obj)) {
        auto* newPlane = new raytracer::Plane(plane->normal, plane->name);
        newPlane->position = plane->position;
        newPlane->color = plane->color;
        newPlane->refractiveIndex = plane->refractiveIndex;
        newPlane->reflectivity = plane->reflectivity;
        return newPlane;
    } else if (auto* disk = dynamic_cast<raytracer::Disk*>(obj)) {
        auto* newDisk = new raytracer::Disk(disk->radius, disk->normal, disk->name);
        newDisk->position = disk->position;
        newDisk->color = disk->color;
        newDisk->refractiveIndex = disk->refractiveIndex;
        newDisk->reflectivity = disk->reflectivity;
        return newDisk;
    } else if (auto* prism = dynamic_cast<raytracer::Prism*>(obj)) {
        auto* newPrism = new raytracer::Prism(prism->size, prism->name);
        newPrism->position = prism->position;
        newPrism->color = prism->color;
        newPrism->refractiveIndex = prism->refractiveIndex;
        newPrism->reflectivity = prism->reflectivity;
        return newPrism;
    } else if (auto* pyramid = dynamic_cast<raytracer::Pyramid*>(obj)) {
        auto* newPyramid = new raytracer::Pyramid(pyramid->baseSize, pyramid->height, pyramid->name);
        newPyramid->position = pyramid->position;
        newPyramid->color = pyramid->color;
        newPyramid->refractiveIndex = pyramid->refractiveIndex;
        newPyramid->reflectivity = pyramid->reflectivity;
        return newPyramid;
    }
    
    return nullptr;
}

EventResult PropertiesWindow::OnKeyDown(KeyEvent& evt) {
    if (evt.key == dr4::KEYCODE_C && (evt.mods & dr4::KEYMOD_CTRL)) {
        if (currentObject) {
            CopyObject();
        }
        return EventResult::HANDLED;
    } else if (evt.key == dr4::KEYCODE_V && (evt.mods & dr4::KEYMOD_CTRL)) {
        if (onPasteRequest) {
            onPasteRequest();
        }
        return EventResult::HANDLED;
    }
    return Container::OnKeyDown(evt);
}

EventResult PropertiesWindow::OnMouseDown(MouseButtonEvent& evt) {
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (evt.pos.x > GetSize().x - 30 && evt.pos.y < 25) {
            isCollapsed = !isCollapsed;
            ForceRedraw();
            return EventResult::HANDLED;
        }
    }
    return Container::OnMouseDown(evt);
}

} // namespace ui

