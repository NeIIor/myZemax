#include "ui/application.hpp"
#include "ui/main_window.hpp"
#include "cum/ifc/dr4.hpp"
#include "raytracer/objects.hpp"
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
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

Application::Application() {
    pluginManager = new cum::Manager();
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
    window->SetSize(dr4::Vec2f(1280, 720));
    
    ui = new hui::UI(window);
    camera = raytracer::Camera(raytracer::Vec3(0, 0, 5), raytracer::Vec3(0, 0, 0), raytracer::Vec3(0, 1, 0));
    raytracer = new raytracer::RayTracer(&scene, &camera);
    
    auto sphere = std::make_unique<raytracer::Sphere>(1.0f, "Sphere 1");
    sphere->position = raytracer::Vec3(0, 0, 0);
    sphere->color = dr4::Color(255, 100, 100);
    scene.AddObject(std::move(sphere));
    
    auto light = std::make_unique<raytracer::Sphere>(0.5f, "Light Source");
    light->position = raytracer::Vec3(3, 3, 3);
    light->isLightSource = true;
    light->color = dr4::Color(255, 255, 255);
    scene.AddObject(std::move(light));
    
    auto* mainWindow = new MainWindow(ui);
    mainWindow->SetSize(window->GetSize());
    mainWindow->SetupLayout(&scene, &camera, raytracer);
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
    std::string pluginsDir = GetPluginsDirectory();
    
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
    std::string backendDir = pluginsDir + "/backend";
    
    if (fs::exists(backendDir) && fs::is_directory(backendDir)) {
        try {
            for (const auto& entry : fs::directory_iterator(backendDir)) {
                if (entry.is_regular_file()) {
                    std::string path = entry.path().string();
                    if (path.size() >= 3 && path.substr(path.size() - 3) == ".so") {
                        pluginPaths.push_back(path);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading backend plugins: " << e.what() << std::endl;
        }
    }
    
    try {
        for (const auto& entry : fs::directory_iterator(pluginsDir)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                if (path.size() >= 3 && path.substr(path.size() - 3) == ".so") {
                    std::string normalized = path;
                    std::replace(normalized.begin(), normalized.end(), '\\', '/');
                    if (normalized.find("/backend/") == std::string::npos) {
                        pluginPaths.push_back(path);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading plugins directory: " << e.what() << std::endl;
    }
    
    for (const auto& path : pluginPaths) {
        try {
            std::cout << "Loading plugin: " << path << std::endl;
            pluginManager->LoadFromFile(path);
        } catch (const cum::Manager::LoadError& e) {
            std::cerr << "Failed to load plugin " << path << ": " << e.what() << std::endl;
        }
    }
    
    try {
        pluginManager->TriggerAfterLoad();
        std::cout << "Loaded " << pluginPaths.size() << " plugin(s) successfully" << std::endl;
    } catch (const cum::Manager::DependencyError& e) {
        std::cerr << "Plugin dependency error: " << e.what() << std::endl;
    }
}

} // namespace ui