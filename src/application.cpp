#include "ui/application.hpp"
#include "ui/main_window.hpp"
#include "cum/ifc/dr4.hpp"
#include "hui/event.hpp"
#include "raytracer/objects.hpp"
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <unistd.h>
#include <limits.h>

namespace ui {

std::string GetExecutablePath() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
        return "";
    }
    return std::string(path, count);
}

std::string GetPluginsDirectory() {
    std::string exePath = GetExecutablePath();
    if (exePath.empty()) {
        return "plugins";
    }
    
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    std::filesystem::path projectPlugins = exeDir.parent_path() / "plugins";
    
    if (std::filesystem::exists(projectPlugins)) {
        return projectPlugins.string();
    }
    
    std::filesystem::path sameDirPlugins = exeDir / "plugins";
    if (std::filesystem::exists(sameDirPlugins)) {
        return sameDirPlugins.string();
    }
    
    return projectPlugins.string();
}

std::string GetFontsDirectory() {
    std::string exePath = GetExecutablePath();
    if (exePath.empty()) {
        return "fonts";
    }

    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    std::filesystem::path projectFonts = exeDir.parent_path() / "fonts";

    if (std::filesystem::exists(projectFonts)) {
        return projectFonts.string();
    }

    std::filesystem::path sameDirFonts = exeDir / "fonts";
    if (std::filesystem::exists(sameDirFonts)) {
        return sameDirFonts.string();
    }

    return projectFonts.string();
}

bool LoadDefaultFont(dr4::Window* window) {
    namespace fs = std::filesystem;
    if (!window) return false;
    std::string fontsDir = GetFontsDirectory();
    if (!fs::exists(fontsDir) || !fs::is_directory(fontsDir)) {
        return false;
    }

    std::vector<std::string> candidates;
    try {
        for (const auto& entry : fs::directory_iterator(fontsDir)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".ttf" || ext == ".otf") {
                candidates.push_back(entry.path().string());
            }
        }
    } catch (...) {
        return false;
    }

    if (candidates.empty()) {
        return false;
    }

    std::sort(candidates.begin(), candidates.end());
    auto font = window->CreateFont();
    try {
        font->LoadFromFile(candidates.front());
        window->SetDefaultFont(font);
        std::cout << "Using font: " << candidates.front() << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load font: " << e.what() << std::endl;
    }
    return false;
}

Application::Application() {
    pluginManager = new cum::Manager();
    pluginsDir = GetPluginsDirectory();
    LoadPlugins();
    
    auto* backend = pluginManager->GetAnyOfType<cum::DR4BackendPlugin>();
    if (!backend) {
        throw std::runtime_error("No DR4 backend plugin loaded! Please place a backend plugin (.so) in the plugins/ directory.");
    }
    
    window = backend->CreateWindow();
    if (!window) {
        throw std::runtime_error("Failed to create window!");
    }
    
    window->SetTitle("myZemax - Optical Constructor");
    window->SetSize(dr4::Vec2f(1600, 900));

    LoadDefaultFont(window);
    
    ui = new hui::UI(window);
    
    camera = raytracer::Camera(
        raytracer::Vec3(0.0f, 3.0f, 12.0f),
        raytracer::Vec3(0.0f, 0.0f, 0.0f),
        raytracer::Vec3(0.0f, 1.0f, 0.0f),
        55.0f
    );
    raytracer = new raytracer::RayTracer(&scene, &camera);

    
    auto ground = std::make_unique<raytracer::Plane>(raytracer::Vec3(0, 1, 0), "Ground");
    ground->position = raytracer::Vec3(0.0f, -2.0f, 0.0f);
    ground->color = dr4::Color(60, 70, 90);
    ground->reflectivity = 0.15f;
    scene.AddObject(std::move(ground));

    
    auto sphereRed = std::make_unique<raytracer::Sphere>(1.0f, "Sphere (red)");
    sphereRed->position = raytracer::Vec3(-2.2f, -1.0f, 0.0f);
    sphereRed->color = dr4::Color(240, 90, 90);
    sphereRed->reflectivity = 0.05f;
    scene.AddObject(std::move(sphereRed));

    auto sphereGlass = std::make_unique<raytracer::Sphere>(0.9f, "Sphere (glass-ish)");
    sphereGlass->position = raytracer::Vec3(1.0f, -1.1f, -1.8f);
    sphereGlass->color = dr4::Color(180, 220, 255);
    sphereGlass->refractiveIndex = 1.45f;
    sphereGlass->reflectivity = 0.10f;
    scene.AddObject(std::move(sphereGlass));

    auto prism = std::make_unique<raytracer::Prism>(raytracer::Vec3(1.6f, 1.6f, 1.6f), "Prism");
    prism->position = raytracer::Vec3(3.0f, -1.2f, 0.8f);
    prism->color = dr4::Color(120, 200, 255);
    prism->reflectivity = 0.25f;
    scene.AddObject(std::move(prism));

    auto pyramid = std::make_unique<raytracer::Pyramid>(2.2f, 2.2f, "Pyramid");
    pyramid->position = raytracer::Vec3(0.0f, -0.9f, 2.4f);
    pyramid->color = dr4::Color(255, 210, 120);
    pyramid->reflectivity = 0.18f;
    scene.AddObject(std::move(pyramid));

    auto disk = std::make_unique<raytracer::Disk>(1.6f, raytracer::Vec3(0, 0, 1), "Disk");
    disk->position = raytracer::Vec3(-4.0f, -0.7f, 1.2f);
    disk->color = dr4::Color(170, 255, 170);
    disk->reflectivity = 0.05f;
    scene.AddObject(std::move(disk));

    
    auto lightWarm = std::make_unique<raytracer::Sphere>(0.35f, "Light (warm)");
    lightWarm->position = raytracer::Vec3(4.5f, 4.5f, 4.0f);
    lightWarm->isLightSource = true;
    lightWarm->color = dr4::Color(255, 200, 170);
    scene.AddObject(std::move(lightWarm));

    auto lightRed = std::make_unique<raytracer::Sphere>(0.30f, "Light (red)");
    lightRed->position = raytracer::Vec3(-5.0f, 3.5f, 2.0f);
    lightRed->isLightSource = true;
    lightRed->color = dr4::Color(255, 90, 90);
    scene.AddObject(std::move(lightRed));

    auto lightBlue = std::make_unique<raytracer::Sphere>(0.30f, "Light (blue)");
    lightBlue->position = raytracer::Vec3(0.0f, 5.5f, -6.0f);
    lightBlue->isLightSource = true;
    lightBlue->color = dr4::Color(90, 120, 255);
    scene.AddObject(std::move(lightBlue));
    
    mainWindow = new MainWindow(ui);
    mainWindow->SetSize(window->GetSize());
    mainWindow->SetupLayout(&scene, &camera, raytracer);
    mainWindow->SetPluginsDirectory(pluginsDir);
    mainWindow->SetPluginManager(pluginManager);
    ui->SetRoot(mainWindow);
}

Application::~Application() {
    delete raytracer;
    delete ui;
    delete window;
    delete pluginManager;
}

void Application::Run() {
    window->Open();
    
    double lastTime = window->GetTime();
    
    while (window->IsOpen()) {
        double currentTime = window->GetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        ProcessEvents();
        Update(deltaTime);
        Render();
        window->Sleep(1.0 / 60.0);
    }
}

void Application::ProcessEvents() {
    std::optional<dr4::Event> event;
    while ((event = window->PollEvent())) {
        dr4::Event dr4Event = *event;
        ui->ProcessEvent(dr4Event);

        
        
        
        
        
        if (dr4Event.type == dr4::Event::Type::KEY_DOWN && mainWindow) {
            if (dr4Event.key.sym == dr4::KeyCode::KEYCODE_F) {
                hui::KeyEvent keyEvent{};
                keyEvent.key = dr4Event.key.sym;
                keyEvent.mods = dr4Event.key.mods;
                keyEvent.pressed = true;
                keyEvent.Apply(*mainWindow);
            }
        }
        
        if (dr4Event.type == dr4::Event::Type::QUIT) {
            window->Close();
        }
    }
}

void Application::Update(float deltaTime) {
    hui::IdleEvent idleEvent;
    idleEvent.absTime = window->GetTime();
    idleEvent.deltaTime = deltaTime;
    ui->OnIdle(idleEvent);
}

void Application::Render() {
    window->Clear(dr4::Color(40, 40, 40));
    dr4::Texture* uiTexture = ui->GetTexture();
    if (uiTexture) {
        window->Draw(*uiTexture);
    }
    
    window->Display();
}

void Application::LoadPlugins() {
    namespace fs = std::filesystem;
    std::string pluginsDir = this->pluginsDir.empty() ? GetPluginsDirectory() : this->pluginsDir;
    
    std::cout << "Looking for plugins in: " << pluginsDir << std::endl;
    
    if (!fs::exists(pluginsDir) || !fs::is_directory(pluginsDir)) {
        std::cerr << "Warning: plugins directory not found. Creating it..." << std::endl;
        try {
            fs::create_directories(pluginsDir);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create plugins directory: " << e.what() << std::endl;
        }
        return;
    }
    
    std::vector<std::string> pluginPaths;
    std::unordered_set<std::string> seen;
    std::string backendDir = pluginsDir + "/backend";
    std::string toolsDir   = pluginsDir + "/tools";
    
    if (fs::exists(backendDir) && fs::is_directory(backendDir)) {
        try {
            for (const auto& entry : fs::directory_iterator(backendDir)) {
                if (entry.is_regular_file()) {
                    std::string path = entry.path().string();
                    if (path.size() >= 3 && path.substr(path.size() - 3) == ".so") {
                        if (seen.insert(path).second)
                            pluginPaths.push_back(path);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading backend plugins: " << e.what() << std::endl;
        }
    }
    

    if (fs::exists(toolsDir) && fs::is_directory(toolsDir)) {
        try {
            
            
            std::string wanted = "libdorisovka_pp.so";
            if (const char* env = std::getenv("MYZEMAX_TOOLS_PLUGIN")) {
                if (std::string(env).size() > 0)
                    wanted = env;
            }
            for (const auto& entry : fs::directory_iterator(toolsDir)) {
                if (entry.is_regular_file()) {
                    std::string path = entry.path().string();
                    if (path.size() >= 3 && path.substr(path.size() - 3) == ".so") {
                        std::string fname = entry.path().filename().string();
                        if (wanted != "*" && fname != wanted) {
                            continue;
                        }
                        if (seen.insert(path).second)
                            pluginPaths.push_back(path);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading tools plugins: " << e.what() << std::endl;
        }
    }
    
    size_t loadedOk = 0;
    for (const auto& path : pluginPaths) {
        try {
            std::cout << "Loading plugin: " << path << std::endl;
            pluginManager->LoadFromFile(path);
            loadedOk++;
        } catch (const cum::Manager::LoadError& e) {
            std::cerr << "Failed to load plugin " << path << ": " << e.what() << std::endl;
        }
    }
    
    try {
        pluginManager->TriggerAfterLoad();
        std::cout << "Loaded " << loadedOk << " / " << pluginPaths.size() << " plugin(s) successfully" << std::endl;
    } catch (const cum::Manager::DependencyError& e) {
        std::cerr << "Plugin dependency error: " << e.what() << std::endl;
    }
}

} // namespace ui