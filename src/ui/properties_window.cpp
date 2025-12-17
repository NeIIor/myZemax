#include "ui/properties_window.hpp"
#include "raytracer/object.hpp"
#include "raytracer/objects.hpp"
#include "raytracer/scene.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include "dr4/keycodes.hpp"
#include "dr4/math/rect.hpp"
#include "hui/event.hpp"
#include "hui/ui.hpp"

namespace ui {

static bool HasSelection(size_t a, size_t b) { return a != b; }

static size_t CaretFromX(dr4::Window* w, float fontSize, const std::string& txt, float localX) {
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

static size_t MaxFieldLen(int idx) {
    if (idx == 0) return 64;
    if (idx >= 1 && idx <= 3) return 16;
    if (idx >= 4 && idx <= 6) return 3;
    if (idx == 7) return 10;
    if (idx == 8) return 6;
    return 16;
}

PropertiesWindow::PropertiesWindow(hui::UI* ui) : Container(ui) {
    SetSize(300, 400);
    keyHeld.fill(false);
}

void PropertiesWindow::SetCollapsed(bool collapsed) {
    if (isCollapsed == collapsed) return;
    isCollapsed = collapsed;
    if (isCollapsed) {
        expandedSize = GetSize();
        hasExpandedSize = true;
        SetSize(GetSize().x, titleBarHeight);
    } else if (hasExpandedSize) {
        SetSize(expandedSize);
    }
    ForceRedraw();
}

hui::EventResult PropertiesWindow::PropagateToChildren(hui::Event& event) {
    return hui::EventResult::UNHANDLED;
}

void PropertiesWindow::SetObject(raytracer::Object* obj) {
    currentObject = obj;
    draftObject.reset();
    draftScene = nullptr;
    currentKind = ObjKind::Unknown;
    activeField = -1;
    caretPos = 0;
    selStart = selEnd = 0;
    selectingText = false;
    selectionDragging = false;
    selectionAnchor = 0;
    selectionMouseDownPos = dr4::Vec2f(0, 0);
    keyHeld.fill(false);
    hoveredButton = ButtonId::None;
    pressedButton = ButtonId::None;
    caretVisible = true;
    caretBlinkAccum = 0.0;
    UpdateFromObject();
    ForceRedraw();
}

void PropertiesWindow::StartDraft(raytracer::Scene* scene, std::unique_ptr<raytracer::Object> obj) {
    draftScene = scene;
    draftObject = std::move(obj);
    currentObject = draftObject.get();
    currentKind = ObjKind::Unknown;
    activeField = -1;
    caretPos = 0;
    selStart = selEnd = 0;
    selectingText = false;
    selectionDragging = false;
    selectionAnchor = 0;
    selectionMouseDownPos = dr4::Vec2f(0, 0);
    keyHeld.fill(false);
    hoveredButton = ButtonId::None;
    pressedButton = ButtonId::None;
    caretVisible = true;
    caretBlinkAccum = 0.0;
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
        sphereRadiusText = "";
        planeNxText = planeNyText = planeNzText = "";
        rectPlaneNxText = rectPlaneNyText = rectPlaneNzText = "";
        rectPlaneWidthText = rectPlaneHeightText = "";
        diskRadiusText = "";
        diskNxText = diskNyText = diskNzText = "";
        prismSizeXText = prismSizeYText = prismSizeZText = "";
        pyramidBaseText = pyramidHeightText = "";
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

    sphereRadiusText.clear();
    planeNxText.clear(); planeNyText.clear(); planeNzText.clear();
    rectPlaneNxText.clear(); rectPlaneNyText.clear(); rectPlaneNzText.clear();
    rectPlaneWidthText.clear(); rectPlaneHeightText.clear();
    diskRadiusText.clear();
    diskNxText.clear(); diskNyText.clear(); diskNzText.clear();
    prismSizeXText.clear(); prismSizeYText.clear(); prismSizeZText.clear();
    pyramidBaseText.clear(); pyramidHeightText.clear();

    currentKind = ObjKind::Unknown;
    if (auto* s = dynamic_cast<raytracer::Sphere*>(currentObject)) {
        currentKind = currentObject->isLightSource ? ObjKind::Light : ObjKind::Sphere;
        oss.str("");
        oss << s->radius;
        sphereRadiusText = oss.str();
    } else if (auto* p = dynamic_cast<raytracer::Plane*>(currentObject)) {
        currentKind = ObjKind::Plane;
        oss.str(""); oss << p->normal.x; planeNxText = oss.str();
        oss.str(""); oss << p->normal.y; planeNyText = oss.str();
        oss.str(""); oss << p->normal.z; planeNzText = oss.str();
    } else if (auto* rp = dynamic_cast<raytracer::RectPlane*>(currentObject)) {
        currentKind = ObjKind::RectPlane;
        oss.str(""); oss << rp->normal.x; rectPlaneNxText = oss.str();
        oss.str(""); oss << rp->normal.y; rectPlaneNyText = oss.str();
        oss.str(""); oss << rp->normal.z; rectPlaneNzText = oss.str();
        oss.str(""); oss << rp->width; rectPlaneWidthText = oss.str();
        oss.str(""); oss << rp->height; rectPlaneHeightText = oss.str();
    } else if (auto* d = dynamic_cast<raytracer::Disk*>(currentObject)) {
        currentKind = ObjKind::Disk;
        oss.str(""); oss << d->radius; diskRadiusText = oss.str();
        oss.str(""); oss << d->normal.x; diskNxText = oss.str();
        oss.str(""); oss << d->normal.y; diskNyText = oss.str();
        oss.str(""); oss << d->normal.z; diskNzText = oss.str();
    } else if (auto* pr = dynamic_cast<raytracer::Prism*>(currentObject)) {
        currentKind = ObjKind::Prism;
        oss.str(""); oss << pr->size.x; prismSizeXText = oss.str();
        oss.str(""); oss << pr->size.y; prismSizeYText = oss.str();
        oss.str(""); oss << pr->size.z; prismSizeZText = oss.str();
    } else if (auto* py = dynamic_cast<raytracer::Pyramid*>(currentObject)) {
        currentKind = ObjKind::Pyramid;
        oss.str(""); oss << py->baseSize; pyramidBaseText = oss.str();
        oss.str(""); oss << py->height; pyramidHeightText = oss.str();
    }
}

int PropertiesWindow::FieldCount() const {
    if (!currentObject) return 0;
    switch (currentKind) {
        case ObjKind::Sphere: return 10;
        case ObjKind::Light: return 10;
        case ObjKind::Plane: return 12;
        case ObjKind::RectPlane: return 14;
        case ObjKind::Disk: return 13;
        case ObjKind::Prism: return 12;
        case ObjKind::Pyramid: return 11;
        default: return 9;
    }
}

float PropertiesWindow::FieldValueOffset(int idx) const {
    if (idx == 7 || idx == 8) return 130.0f;
    return 80.0f;
}

void PropertiesWindow::Redraw() const {
    dr4::Texture& texture = GetTexture();
    const dr4::Color bgPanel(64, 64, 66);
    const dr4::Color bgHeader(76, 76, 76);
    const dr4::Color borderHeader(96, 96, 96);
    const dr4::Color textMain(235, 235, 235);
    const dr4::Color textSub(210, 210, 210);
    const dr4::Color fieldBg(74, 74, 76);
    const dr4::Color fieldActive(82, 92, 116);
    const dr4::Color btnHover(88, 88, 92);
    const dr4::Color btnPressed(62, 62, 64);
    texture.Clear(isCollapsed ? dr4::Color(0, 0, 0, 0) : bgPanel);
    
    auto* bar = GetUI()->GetWindow()->CreateRectangle();
    bar->SetPos(dr4::Vec2f(0, 0));
    bar->SetSize(dr4::Vec2f(GetSize().x, titleBarHeight));
    bar->SetFillColor(bgHeader);
    bar->SetBorderColor(borderHeader);
    bar->SetBorderThickness(1.0f);
    texture.Draw(*bar);
    
    auto* titleText = GetUI()->GetWindow()->CreateText();
    titleText->SetText(draftObject ? "Properties (new)" : "Properties");
    titleText->SetPos(dr4::Vec2f(12, 6));
    titleText->SetFontSize(14);
    titleText->SetColor(textMain);
    texture.Draw(*titleText);
    
    auto* arrowText = GetUI()->GetWindow()->CreateText();
    arrowText->SetText(isCollapsed ? ">" : "v");
    arrowText->SetPos(dr4::Vec2f(GetSize().x - 24, 6));
    arrowText->SetFontSize(14);
    arrowText->SetColor(textMain);
    texture.Draw(*arrowText);
    
    if (isCollapsed) return;
    
    if (!currentObject) {
        auto* text = GetUI()->GetWindow()->CreateText();
        text->SetText("No object selected");
        text->SetPos(dr4::Vec2f(10, titleBarHeight + 4));
        text->SetFontSize(14);
        text->SetColor(textSub);
        texture.Draw(*text);
        return;
    }
    
    float y = titleBarHeight + 6.0f;
    float lineHeight = 27.0f;
    
    auto* label = GetUI()->GetWindow()->CreateText();
    auto* value = GetUI()->GetWindow()->CreateText();
    label->SetFontSize(13);
    value->SetFontSize(13);

    auto drawField = [&](const std::string& fieldLabel, const std::string& fieldValue, int idx, float xValueOffset = 80.0f) {
        label->SetText(fieldLabel);
        label->SetPos(dr4::Vec2f(10, y));
        label->SetColor(textMain);
        texture.Draw(*label);

        
        
        float labelW = label->GetBounds().x;
        float neededOffset = 10.0f + labelW + 14.0f; 
        float minFieldW = 90.0f;
        float maxOffset = std::max(xValueOffset, GetSize().x - minFieldW);
        float xOff = std::min(std::max(xValueOffset, neededOffset), maxOffset);

        auto* rect = GetUI()->GetWindow()->CreateRectangle();
        rect->SetPos(dr4::Vec2f(xOff - 5, y));
        rect->SetSize(dr4::Vec2f(GetSize().x - xOff - 14, lineHeight - 8));
        rect->SetFillColor(idx == activeField ? fieldActive : fieldBg);
        rect->SetBorderColor(borderHeader);
        rect->SetBorderThickness(1.0f);
        texture.Draw(*rect);

        value->SetText(fieldValue);
        value->SetPos(dr4::Vec2f(xOff, y - 1));
        value->SetColor(idx == activeField ? dr4::Color(255, 230, 140) : textSub);
    texture.Draw(*value);
    
        if (idx == activeField && HasSelection(selStart, selEnd)) {
            size_t a = std::min(selStart, selEnd);
            size_t b = std::max(selStart, selEnd);
            a = std::min(a, fieldValue.size());
            b = std::min(b, fieldValue.size());
            auto* m1 = GetUI()->GetWindow()->CreateText();
            m1->SetFontSize(13);
            m1->SetText(fieldValue.substr(0, a));
            float w1 = m1->GetBounds().x;
            auto* m2 = GetUI()->GetWindow()->CreateText();
            m2->SetFontSize(13);
            m2->SetText(fieldValue.substr(0, b));
            float w2 = m2->GetBounds().x;
            float sx = xOff + w1;
            float ex = xOff + w2;
            float left = std::min(sx, ex);
            float right = std::max(sx, ex);
            auto* sel = GetUI()->GetWindow()->CreateRectangle();
            sel->SetPos(dr4::Vec2f(left, y - 1));
            sel->SetSize(dr4::Vec2f(std::max(0.0f, right - left), 16.0f));
            sel->SetFillColor(dr4::Color(120, 170, 230, 140));
            texture.Draw(*sel);
    texture.Draw(*value);
        }
        if (idx == activeField && caretVisible) {
            auto* measure = GetUI()->GetWindow()->CreateText();
            measure->SetFontSize(13);
            size_t cp = std::min(caretPos, fieldValue.size());
            measure->SetText(fieldValue.substr(0, cp));
            float w = measure->GetBounds().x;
            float cx = xOff + w;
            float left = xOff;
            float right = xOff + (GetSize().x - xOff - 14) - 4.0f;
            cx = std::clamp(cx, left, right);
            auto* caret = GetUI()->GetWindow()->CreateRectangle();
            caret->SetPos(dr4::Vec2f(cx, y + 2.0f));
            caret->SetSize(dr4::Vec2f(1.0f, 16.0f));
            caret->SetFillColor(dr4::Color(255, 230, 140));
            texture.Draw(*caret);
        }
    
    y += lineHeight;
    };

    drawField("Name:", nameText, 0, FieldValueOffset(0));
    drawField("Position X:", posXText, 1, FieldValueOffset(1));
    drawField("Position Y:", posYText, 2, FieldValueOffset(2));
    drawField("Position Z:", posZText, 3, FieldValueOffset(3));
    drawField("Color R:", colorRText, 4, FieldValueOffset(4));
    drawField("Color G:", colorGText, 5, FieldValueOffset(5));
    drawField("Color B:", colorBText, 6, FieldValueOffset(6));
    drawField("Refractive Index:", refractiveIndexText, 7, FieldValueOffset(7));
    drawField("Reflectivity:", reflectivityText, 8, FieldValueOffset(8));

    int fc = FieldCount();
    if (fc > 9) {
        if (currentKind == ObjKind::Sphere || currentKind == ObjKind::Light) {
            drawField("Radius:", sphereRadiusText, 9, FieldValueOffset(9));
        } else if (currentKind == ObjKind::Plane) {
            drawField("Normal X:", planeNxText, 9, FieldValueOffset(9));
            drawField("Normal Y:", planeNyText, 10, FieldValueOffset(10));
            drawField("Normal Z:", planeNzText, 11, FieldValueOffset(11));
        } else if (currentKind == ObjKind::RectPlane) {
            drawField("Normal X:", rectPlaneNxText, 9, FieldValueOffset(9));
            drawField("Normal Y:", rectPlaneNyText, 10, FieldValueOffset(10));
            drawField("Normal Z:", rectPlaneNzText, 11, FieldValueOffset(11));
            drawField("Width:", rectPlaneWidthText, 12, FieldValueOffset(12));
            drawField("Height:", rectPlaneHeightText, 13, FieldValueOffset(13));
        } else if (currentKind == ObjKind::Disk) {
            drawField("Radius:", diskRadiusText, 9, FieldValueOffset(9));
            drawField("Normal X:", diskNxText, 10, FieldValueOffset(10));
            drawField("Normal Y:", diskNyText, 11, FieldValueOffset(11));
            drawField("Normal Z:", diskNzText, 12, FieldValueOffset(12));
        } else if (currentKind == ObjKind::Prism) {
            drawField("Size X:", prismSizeXText, 9, FieldValueOffset(9));
            drawField("Size Y:", prismSizeYText, 10, FieldValueOffset(10));
            drawField("Size Z:", prismSizeZText, 11, FieldValueOffset(11));
        } else if (currentKind == ObjKind::Pyramid) {
            drawField("Base size:", pyramidBaseText, 9, FieldValueOffset(9));
            drawField("Height:", pyramidHeightText, 10, FieldValueOffset(10));
        }
    }

    y += 5.0f;

    auto drawButton = [&](ButtonId id, float x, float w, const char* caption, float textX) {
        dr4::Color fill = fieldBg;
        if (pressedButton == id) fill = btnPressed;
        else if (hoveredButton == id) fill = btnHover;

        auto* rect = GetUI()->GetWindow()->CreateRectangle();
        rect->SetPos(dr4::Vec2f(x, y));
        rect->SetSize(dr4::Vec2f(w, 22));
        rect->SetFillColor(fill);
        rect->SetBorderColor(borderHeader);
        rect->SetBorderThickness(1.0f);
        texture.Draw(*rect);

        auto* text = GetUI()->GetWindow()->CreateText();
        text->SetText(caption);
        float dy = (pressedButton == id) ? 5.0f : 4.0f;
        text->SetPos(dr4::Vec2f(textX, y + dy));
        text->SetFontSize(12);
        text->SetColor(textMain);
        texture.Draw(*text);
    };

    drawButton(ButtonId::Copy, 10, 80, "Copy", 20);
    drawButton(ButtonId::Paste, 100, 80, "Paste", 110);
    drawButton(ButtonId::Enter, 190, 90, draftObject ? "Enter/Add" : "Enter", 202);
}

hui::EventResult PropertiesWindow::OnText(hui::TextEvent& evt) {
    ApplyTextInput(evt.text);
    return hui::EventResult::HANDLED;
}

hui::EventResult PropertiesWindow::OnIdle(hui::IdleEvent& evt) {
    if (activeField >= 0) {
        caretBlinkAccum += evt.deltaTime;
        if (caretBlinkAccum >= 0.5) {
            caretBlinkAccum = 0.0;
            caretVisible = !caretVisible;
            ForceRedraw();
        }
    }
    return Container::OnIdle(evt);
}

void PropertiesWindow::ParseAndApplyChanges() {
    if (!currentObject) return;
    
            currentObject->name = nameText;

    try { currentObject->position.x = std::stof(posXText); } catch (...) {}
    try { currentObject->position.y = std::stof(posYText); } catch (...) {}
    try { currentObject->position.z = std::stof(posZText); } catch (...) {}

    try { int r = std::stoi(colorRText); currentObject->color.r = static_cast<uint8_t>(std::max(0, std::min(255, r))); } catch (...) {}
    try { int g = std::stoi(colorGText); currentObject->color.g = static_cast<uint8_t>(std::max(0, std::min(255, g))); } catch (...) {}
    try { int b = std::stoi(colorBText); currentObject->color.b = static_cast<uint8_t>(std::max(0, std::min(255, b))); } catch (...) {}

    try { currentObject->refractiveIndex = std::stof(refractiveIndexText); } catch (...) {}
    try {
            currentObject->reflectivity = std::stof(reflectivityText);
            currentObject->reflectivity = std::max(0.0f, std::min(1.0f, currentObject->reflectivity));
    } catch (...) {}

    if (auto* s = dynamic_cast<raytracer::Sphere*>(currentObject)) {
        try {
            float r = std::stof(sphereRadiusText.empty() ? "0" : sphereRadiusText);
            s->radius = std::max(0.01f, r);
        } catch (...) {}
    } else if (auto* p = dynamic_cast<raytracer::Plane*>(currentObject)) {
        try { p->normal.x = std::stof(planeNxText.empty() ? "0" : planeNxText); } catch (...) {}
        try { p->normal.y = std::stof(planeNyText.empty() ? "1" : planeNyText); } catch (...) {}
        try { p->normal.z = std::stof(planeNzText.empty() ? "0" : planeNzText); } catch (...) {}
        float len = p->normal.Length();
        if (len > 1e-6f) p->normal = p->normal / len;
    } else if (auto* rp = dynamic_cast<raytracer::RectPlane*>(currentObject)) {
        try { rp->normal.x = std::stof(rectPlaneNxText.empty() ? "0" : rectPlaneNxText); } catch (...) {}
        try { rp->normal.y = std::stof(rectPlaneNyText.empty() ? "1" : rectPlaneNyText); } catch (...) {}
        try { rp->normal.z = std::stof(rectPlaneNzText.empty() ? "0" : rectPlaneNzText); } catch (...) {}
        float len = rp->normal.Length();
        if (len > 1e-6f) rp->normal = rp->normal / len;
        try { rp->width = std::max(0.01f, std::stof(rectPlaneWidthText.empty() ? "1" : rectPlaneWidthText)); } catch (...) {}
        try { rp->height = std::max(0.01f, std::stof(rectPlaneHeightText.empty() ? "1" : rectPlaneHeightText)); } catch (...) {}
    } else if (auto* d = dynamic_cast<raytracer::Disk*>(currentObject)) {
        try {
            float r = std::stof(diskRadiusText.empty() ? "0" : diskRadiusText);
            d->radius = std::max(0.01f, r);
        } catch (...) {}
        try { d->normal.x = std::stof(diskNxText.empty() ? "0" : diskNxText); } catch (...) {}
        try { d->normal.y = std::stof(diskNyText.empty() ? "1" : diskNyText); } catch (...) {}
        try { d->normal.z = std::stof(diskNzText.empty() ? "0" : diskNzText); } catch (...) {}
        float len = d->normal.Length();
        if (len > 1e-6f) d->normal = d->normal / len;
    } else if (auto* pr = dynamic_cast<raytracer::Prism*>(currentObject)) {
        try { pr->size.x = std::max(0.01f, std::stof(prismSizeXText.empty() ? "1" : prismSizeXText)); } catch (...) {}
        try { pr->size.y = std::max(0.01f, std::stof(prismSizeYText.empty() ? "1" : prismSizeYText)); } catch (...) {}
        try { pr->size.z = std::max(0.01f, std::stof(prismSizeZText.empty() ? "1" : prismSizeZText)); } catch (...) {}
    } else if (auto* py = dynamic_cast<raytracer::Pyramid*>(currentObject)) {
        try { py->baseSize = std::max(0.01f, std::stof(pyramidBaseText.empty() ? "1" : pyramidBaseText)); } catch (...) {}
        try { py->height = std::max(0.01f, std::stof(pyramidHeightText.empty() ? "1" : pyramidHeightText)); } catch (...) {}
    }

    if (draftObject && draftScene && currentObject == draftObject.get()) {
        auto makeUnique = [this](const std::string& base) {
            if (!draftScene) return base;
            int counter = 1;
            std::string candidate = base;
            while (draftScene->FindObjectByName(candidate)) {
                candidate = base + " " + std::to_string(counter++);
            }
            return candidate;
        };
        currentObject->name = makeUnique(currentObject->name.empty() ? "Object" : currentObject->name);

        raytracer::Object* raw = draftObject.get();
        draftScene->AddObject(std::move(draftObject));
        draftScene = nullptr;
        currentObject = raw;
        if (onObjectCommitted) onObjectCommitted(raw);
        UpdateFromObject();
    }

    if (onObjectChanged) {
        onObjectChanged();
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
    if (onObjectChanged) {
        onObjectChanged();
    }
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
    } else if (auto* rp = dynamic_cast<raytracer::RectPlane*>(obj)) {
        auto* newRect = new raytracer::RectPlane(rp->width, rp->height, rp->normal, rp->name);
        newRect->position = rp->position;
        newRect->color = rp->color;
        newRect->refractiveIndex = rp->refractiveIndex;
        newRect->reflectivity = rp->reflectivity;
        return newRect;
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

hui::EventResult PropertiesWindow::OnKeyDown(hui::KeyEvent& evt) {
    bool ctrl = (evt.mods & dr4::KEYMOD_CTRL) != 0;

    const int keyIdx = static_cast<int>(evt.key) + 1;
    const bool hasKeyIdx = (keyIdx >= 0 && keyIdx < static_cast<int>(keyHeld.size()));
    if (activeField >= 0) {
        if (hasKeyIdx && keyHeld[keyIdx]) {
            return hui::EventResult::HANDLED;
        }
        if (hasKeyIdx) keyHeld[keyIdx] = true;
    }

    if (activeField >= 0 && ctrl && evt.key == dr4::KEYCODE_C) {
        if (auto* field = GetFieldByIndex(activeField)) {
            size_t a = std::min(selStart, selEnd);
            size_t b = std::max(selStart, selEnd);
            a = std::min(a, field->size());
            b = std::min(b, field->size());
            std::string toCopy = HasSelection(selStart, selEnd) ? field->substr(a, b - a) : *field;
            if (GetUI() && GetUI()->GetWindow()) GetUI()->GetWindow()->SetClipboard(toCopy);
        }
        return hui::EventResult::HANDLED;
        }
    if (activeField >= 0 && ctrl && evt.key == dr4::KEYCODE_V) {
        std::string clip;
        if (GetUI() && GetUI()->GetWindow()) clip = GetUI()->GetWindow()->GetClipboard();
        ApplyTextInput(clip.c_str());
        return hui::EventResult::HANDLED;
    }
    if (activeField < 0 && ctrl && evt.key == dr4::KEYCODE_C) { if (currentObject) CopyObject(); return hui::EventResult::HANDLED; }
    if (activeField < 0 && ctrl && evt.key == dr4::KEYCODE_V) { if (onPasteRequest) onPasteRequest(); return hui::EventResult::HANDLED; }
    if (activeField >= 0) {
        if (auto* field = GetFieldByIndex(activeField)) {
            caretPos = std::min(caretPos, field->size());
        }
        if (evt.key == dr4::KEYCODE_LEFT) {
            if (caretPos > 0) caretPos--;
            selStart = selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KEYCODE_RIGHT) {
            if (auto* field = GetFieldByIndex(activeField)) {
                if (caretPos < field->size()) caretPos++;
            }
            selStart = selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KEYCODE_HOME) {
            caretPos = 0;
            selStart = selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KEYCODE_END) {
            if (auto* field = GetFieldByIndex(activeField)) caretPos = field->size();
            selStart = selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KEYCODE_BACKSPACE) {
            if (auto* field = GetFieldByIndex(activeField)) {
                if (HasSelection(selStart, selEnd)) {
                    size_t a = std::min(selStart, selEnd);
                    size_t b = std::max(selStart, selEnd);
                    a = std::min(a, field->size());
                    b = std::min(b, field->size());
                    field->erase(a, b - a);
                    caretPos = a;
                } else if (caretPos > 0 && caretPos <= field->size()) {
                    field->erase(caretPos - 1, 1);
                    caretPos--;
                }
            }
            selStart = selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KEYCODE_DELETE) {
            if (auto* field = GetFieldByIndex(activeField)) {
                if (HasSelection(selStart, selEnd)) {
                    size_t a = std::min(selStart, selEnd);
                    size_t b = std::max(selStart, selEnd);
                    a = std::min(a, field->size());
                    b = std::min(b, field->size());
                    field->erase(a, b - a);
                    caretPos = a;
                } else if (caretPos < field->size()) {
                    field->erase(caretPos, 1);
                }
            }
            selStart = selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KEYCODE_ENTER) {
            ParseAndApplyChanges();
            return hui::EventResult::HANDLED;
        }
        if (evt.key == dr4::KEYCODE_TAB) {
            int fc = std::max(1, FieldCount());
            activeField = (activeField + 1) % fc;
            if (auto* field = GetFieldByIndex(activeField)) caretPos = field->size();
            selStart = selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
    }
    return Container::OnKeyDown(evt);
}

hui::EventResult PropertiesWindow::OnKeyUp(hui::KeyEvent& evt) {
    const int keyIdx = static_cast<int>(evt.key) + 1;
    if (keyIdx >= 0 && keyIdx < static_cast<int>(keyHeld.size())) {
        keyHeld[keyIdx] = false;
    }
    return Container::OnKeyUp(evt);
}

hui::EventResult PropertiesWindow::OnMouseDown(hui::MouseButtonEvent& evt) {
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (evt.pos.y <= titleBarHeight) {
            if (evt.pos.x > GetSize().x - 30) {
                SetCollapsed(!isCollapsed);
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

        float y = titleBarHeight + 6.0f;
        float lineHeight = 27.0f;
        int fc = FieldCount();
        for (int idx = 0; idx < fc; ++idx) {
            float fieldY = y + idx * lineHeight;
            float xValueOffset = FieldValueOffset(idx);
            dr4::Rect2f fieldRect(dr4::Vec2f(xValueOffset - 5, fieldY), dr4::Vec2f(GetSize().x - xValueOffset - 10, lineHeight - 6));
            if (fieldRect.Contains(evt.pos)) {
                activeField = idx;
                if (auto* field = GetFieldByIndex(activeField)) {
                    float localX = evt.pos.x - xValueOffset;
                    caretPos = CaretFromX(GetUI()->GetWindow(), 13.0f, *field, localX);
                } else {
                    caretPos = 0;
                }
                selStart = selEnd = caretPos;
                selectingText = true;
                selectionDragging = false;
                selectionAnchor = caretPos;
                selectionMouseDownPos = evt.pos;
                caretVisible = true;
                caretBlinkAccum = 0.0;
                GetUI()->ReportFocus(this);
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }


        float buttonsY = y + static_cast<float>(fc) * lineHeight + 5.0f;
        dr4::Rect2f copyRect(dr4::Vec2f(10, buttonsY), dr4::Vec2f(80, 22));
        dr4::Rect2f pasteRect(dr4::Vec2f(100, buttonsY), dr4::Vec2f(80, 22));
        dr4::Rect2f applyRect(dr4::Vec2f(190, buttonsY), dr4::Vec2f(90, 22));

        pressedButton = ButtonId::None;
        if (copyRect.Contains(evt.pos)) pressedButton = ButtonId::Copy;
        else if (pasteRect.Contains(evt.pos)) pressedButton = ButtonId::Paste;
        else if (applyRect.Contains(evt.pos)) pressedButton = ButtonId::Enter;

        if (pressedButton != ButtonId::None) {
            GetUI()->ReportFocus(this);
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
    }
    return Container::OnMouseDown(evt);
}

hui::EventResult PropertiesWindow::OnMouseMove(hui::MouseMoveEvent& evt) {
    GetUI()->ReportHover(this);
    if (dragging) {
        dr4::Vec2f delta = evt.pos - dragOffset;
        SetPos(GetPos() + delta);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }
    if (selectingText && activeField >= 0 && !isCollapsed) {
        constexpr float kDragThresholdPx = 3.0f;
        dr4::Vec2f d = evt.pos - selectionMouseDownPos;
        float dist2 = d.x * d.x + d.y * d.y;
        if (!selectionDragging && dist2 < kDragThresholdPx * kDragThresholdPx) {
            return hui::EventResult::HANDLED;
        }
        selectionDragging = true;
        if (auto* field = GetFieldByIndex(activeField)) {
            float xValueOffset = FieldValueOffset(activeField);
            float localX = evt.pos.x - xValueOffset;
            caretPos = CaretFromX(GetUI()->GetWindow(), 13.0f, *field, localX);
            selStart = selectionAnchor;
            selEnd = caretPos;
            caretVisible = true;
            caretBlinkAccum = 0.0;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
    }

    if (!isCollapsed && currentObject) {
        float y0 = titleBarHeight + 6.0f;
        float lineHeight = 27.0f;
        int fc = FieldCount();
        float buttonsY = y0 + static_cast<float>(fc) * lineHeight + 5.0f;
        dr4::Rect2f copyRect(dr4::Vec2f(10, buttonsY), dr4::Vec2f(80, 22));
        dr4::Rect2f pasteRect(dr4::Vec2f(100, buttonsY), dr4::Vec2f(80, 22));
        dr4::Rect2f applyRect(dr4::Vec2f(190, buttonsY), dr4::Vec2f(90, 22));
        ButtonId newHover = ButtonId::None;
        if (copyRect.Contains(evt.pos)) newHover = ButtonId::Copy;
        else if (pasteRect.Contains(evt.pos)) newHover = ButtonId::Paste;
        else if (applyRect.Contains(evt.pos)) newHover = ButtonId::Enter;
        if (newHover != hoveredButton) {
            hoveredButton = newHover;
            ForceRedraw();
        }
    } else {
        if (hoveredButton != ButtonId::None) {
            hoveredButton = ButtonId::None;
            ForceRedraw();
        }
    }
    return Container::OnMouseMove(evt);
}

hui::EventResult PropertiesWindow::OnMouseUp(hui::MouseButtonEvent& evt) {
    if (dragging) {
        dragging = false;
        return hui::EventResult::HANDLED;
    }
    if (evt.button == dr4::MouseButtonType::LEFT) {
        if (selectingText && !selectionDragging) {
            selStart = selEnd = caretPos;
        }
        selectingText = false;
        selectionDragging = false;

        if (!isCollapsed && currentObject && pressedButton != ButtonId::None) {
            float y0 = titleBarHeight + 6.0f;
            float lineHeight = 27.0f;
            int fc = FieldCount();
            float buttonsY = y0 + static_cast<float>(fc) * lineHeight + 5.0f;
            dr4::Rect2f copyRect(dr4::Vec2f(10, buttonsY), dr4::Vec2f(80, 22));
            dr4::Rect2f pasteRect(dr4::Vec2f(100, buttonsY), dr4::Vec2f(80, 22));
            dr4::Rect2f applyRect(dr4::Vec2f(190, buttonsY), dr4::Vec2f(90, 22));

            bool fire = false;
            if (pressedButton == ButtonId::Copy && copyRect.Contains(evt.pos)) fire = true;
            else if (pressedButton == ButtonId::Paste && pasteRect.Contains(evt.pos)) fire = true;
            else if (pressedButton == ButtonId::Enter && applyRect.Contains(evt.pos)) fire = true;

            ButtonId toFire = pressedButton;
            pressedButton = ButtonId::None;
            ForceRedraw();

            if (fire) {
                if (toFire == ButtonId::Copy) CopyObject();
                else if (toFire == ButtonId::Paste) { if (onPasteRequest) onPasteRequest(); }
                else if (toFire == ButtonId::Enter) ParseAndApplyChanges();
                return hui::EventResult::HANDLED;
            }
            return hui::EventResult::HANDLED;
        }
    }
    return Container::OnMouseUp(evt);
}

std::string* PropertiesWindow::GetFieldByIndex(int idx) {
    switch (idx) {
        case 0: return &nameText;
        case 1: return &posXText;
        case 2: return &posYText;
        case 3: return &posZText;
        case 4: return &colorRText;
        case 5: return &colorGText;
        case 6: return &colorBText;
        case 7: return &refractiveIndexText;
        case 8: return &reflectivityText;
        case 9:
            if (currentKind == ObjKind::Sphere || currentKind == ObjKind::Light) return &sphereRadiusText;
            if (currentKind == ObjKind::Plane) return &planeNxText;
            if (currentKind == ObjKind::RectPlane) return &rectPlaneNxText;
            if (currentKind == ObjKind::Disk) return &diskRadiusText;
            if (currentKind == ObjKind::Prism) return &prismSizeXText;
            if (currentKind == ObjKind::Pyramid) return &pyramidBaseText;
            return nullptr;
        case 10:
            if (currentKind == ObjKind::Plane) return &planeNyText;
            if (currentKind == ObjKind::RectPlane) return &rectPlaneNyText;
            if (currentKind == ObjKind::Disk) return &diskNxText;
            if (currentKind == ObjKind::Prism) return &prismSizeYText;
            if (currentKind == ObjKind::Pyramid) return &pyramidHeightText;
            return nullptr;
        case 11:
            if (currentKind == ObjKind::Plane) return &planeNzText;
            if (currentKind == ObjKind::RectPlane) return &rectPlaneNzText;
            if (currentKind == ObjKind::Disk) return &diskNyText;
            if (currentKind == ObjKind::Prism) return &prismSizeZText;
            return nullptr;
        case 12:
            if (currentKind == ObjKind::RectPlane) return &rectPlaneWidthText;
            if (currentKind == ObjKind::Disk) return &diskNzText;
            return nullptr;
        case 13:
            if (currentKind == ObjKind::RectPlane) return &rectPlaneHeightText;
            return nullptr;
        default: return nullptr;
    }
}

bool PropertiesWindow::IsNumericField(int idx) const {
    return idx >= 1;
}

void PropertiesWindow::ApplyTextInput(const char* text) {
    if (!text || activeField < 0) return;
    std::string* field = GetFieldByIndex(activeField);
    if (!field) return;
    caretPos = std::min(caretPos, field->size());
    if (HasSelection(selStart, selEnd)) {
        size_t a = std::min(selStart, selEnd);
        size_t b = std::max(selStart, selEnd);
        a = std::min(a, field->size());
        b = std::min(b, field->size());
        field->erase(a, b - a);
        caretPos = a;
        selStart = selEnd = caretPos;
    }

    for (const char* p = text; *p; ++p) {
        if (field->size() >= MaxFieldLen(activeField)) break;
        char c = *p;
        if (IsNumericField(activeField)) {
            if (c >= '0' && c <= '9') {
                field->insert(field->begin() + static_cast<long>(std::min(caretPos, field->size())), c);
                caretPos++;
            } else if (c == '.' && field->find('.') == std::string::npos) {
                field->insert(field->begin() + static_cast<long>(std::min(caretPos, field->size())), c);
                caretPos++;
            } else if ((c == '-' || c == '+') && caretPos == 0 && field->find('-') == std::string::npos && field->find('+') == std::string::npos) {
                field->insert(field->begin(), c);
                caretPos = 1;
            }
        } else {
            if (std::isprint(static_cast<unsigned char>(c))) {
                field->insert(field->begin() + static_cast<long>(std::min(caretPos, field->size())), c);
                caretPos++;
            }
        }
    }
    caretVisible = true;
    caretBlinkAccum = 0.0;
    selStart = selEnd = caretPos;
    ForceRedraw();
}

} // namespace ui