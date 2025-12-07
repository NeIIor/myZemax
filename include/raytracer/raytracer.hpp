#ifndef RAYTRACER_RAYTRACER_HPP
#define RAYTRACER_RAYTRACER_HPP

#include <algorithm>
#include "raytracer/scene.hpp"
#include "raytracer/camera.hpp"
#include "raytracer/ray.hpp"
#include "dr4/math/color.hpp"
#include "dr4/texture.hpp"

namespace raytracer {

class RayTracer {
public:
    Scene* scene;
    Camera* camera;
    int maxBounces = 3;
    int samplesPerPixel = 1;

    RayTracer(Scene* scene_, Camera* camera_)
        : scene(scene_), camera(camera_) {}

    dr4::Color TraceRay(const Ray& ray, int depth = 0) {
        if (depth >= maxBounces) {
            return dr4::Color(0, 0, 0);
        }

        HitResult closestHit;
        closestHit.t = 1e10f;

        for (auto& obj : scene->objects) {
            HitResult hit = obj->Intersect(ray);
            if (hit.hit && hit.t < closestHit.t && hit.t > 0.001f) {
                closestHit = hit;
            }
        }

        if (!closestHit.hit) {
            return dr4::Color(20, 20, 30); 
        }

        const Object* obj = closestHit.object;
        
        if (obj->isLightSource) {
            return dr4::Color(255, 255, 255);
        }

        dr4::Color color = obj->color;
        Vec3 lightDir = Vec3(0, 1, 1).Normalized();
        float dot = std::max(0.0f, closestHit.normal.Dot(lightDir));
        float brightness = 0.3f + 0.7f * dot;

        color.r = static_cast<uint8_t>(std::min(255.0f, color.r * brightness));
        color.g = static_cast<uint8_t>(std::min(255.0f, color.g * brightness));
        color.b = static_cast<uint8_t>(std::min(255.0f, color.b * brightness));

        return color;
    }

    void Render(dr4::Image* image) {
        if (!image || !scene || !camera) return;

        float width = image->GetWidth();
        float height = image->GetHeight();

        for (int y = 0; y < static_cast<int>(height); ++y) {
            for (int x = 0; x < static_cast<int>(width); ++x) {
                Ray ray = camera->GetRay(x + 0.5f, y + 0.5f, width, height);
                dr4::Color color = TraceRay(ray);
                image->SetPixel(x, y, color);
            }
        }
    }
};

} // namespace raytracer

#endif // RAYTRACER_RAYTRACER_HPP

