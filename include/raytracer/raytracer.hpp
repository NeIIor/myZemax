#ifndef RAYTRACER_RAYTRACER_HPP
#define RAYTRACER_RAYTRACER_HPP

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>
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

    dr4::Color TraceRay(const Ray& ray, const std::vector<const Object*>& lights, int depth = 0) {
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
            return dr4::Color(15, 17, 28); 
        }

        const Object* obj = closestHit.object;
        
        if (obj->isLightSource) {
            return dr4::Color(255, 255, 255);
        }

        float ambient = 0.45f;
        float r = obj->color.r * ambient;
        float g = obj->color.g * ambient;
        float b = obj->color.b * ambient;


        Vec3 dirLight = Vec3(0.3f, 0.8f, 0.5f).Normalized();
        float ndotlDir = std::max(0.0f, closestHit.normal.Dot(dirLight));
        float dirStrength = 0.35f * ndotlDir;
        r += obj->color.r * dirStrength;
        g += obj->color.g * dirStrength;
        b += obj->color.b * dirStrength;


        for (const Object* light : lights) {
            Vec3 lightPos = light->position;
            Vec3 toLight = lightPos - closestHit.point;
            float dist = toLight.Length();
            Vec3 lightDir = toLight / std::max(1e-4f, dist);


            Ray shadowRay(closestHit.point + closestHit.normal * 0.01f, lightDir);
            bool occluded = false;
            for (auto& objCheck : scene->objects) {
                if (objCheck.get() == obj) continue;
                if (objCheck->isLightSource) continue;
                HitResult shadowHit = objCheck->Intersect(shadowRay);
                if (shadowHit.hit && shadowHit.t > 0.001f && shadowHit.t < dist) {
                    occluded = true;
                    break;
                }
            }
            if (occluded) continue;

            float ndotl = std::max(0.0f, closestHit.normal.Dot(lightDir));

            float atten = 1.0f / (1.0f + 0.02f * dist);
            float diff = ndotl * atten * 1.4f;

            r += obj->color.r * diff;
            g += obj->color.g * diff;
            b += obj->color.b * diff;
        }

        r = std::min(255.0f, r);
        g = std::min(255.0f, g);
        b = std::min(255.0f, b);

        return dr4::Color(static_cast<uint8_t>(r),
                          static_cast<uint8_t>(g),
                          static_cast<uint8_t>(b));
    }

    void Render(dr4::Image* image) {
        if (!image || !scene || !camera) return;

        const int width = static_cast<int>(image->GetWidth());
        const int height = static_cast<int>(image->GetHeight());
        if (width <= 0 || height <= 0) return;

        
        std::vector<const Object*> lights;
        lights.reserve(scene->objects.size());
        for (auto& o : scene->objects) {
            if (o->isLightSource) lights.push_back(o.get());
        }

        
        std::vector<dr4::Color> buffer(static_cast<size_t>(width) * static_cast<size_t>(height));

        const unsigned hw = std::max(1u, std::thread::hardware_concurrency());
        const unsigned workers = std::min<unsigned>(hw, static_cast<unsigned>(height));
        const int chunkRows = 8;
        std::atomic<int> nextY{0};

        auto workerFn = [&]() {
            while (true) {
                int y0 = nextY.fetch_add(chunkRows, std::memory_order_relaxed);
                if (y0 >= height) break;
                int y1 = std::min(height, y0 + chunkRows);
                for (int y = y0; y < y1; ++y) {
                    size_t rowOff = static_cast<size_t>(y) * static_cast<size_t>(width);
                    for (int x = 0; x < width; ++x) {
                        Ray ray = camera->GetRay(x + 0.5f, y + 0.5f,
                                                 static_cast<float>(width), static_cast<float>(height));
                        buffer[rowOff + static_cast<size_t>(x)] = TraceRay(ray, lights);
                    }
                }
            }
        };

        std::vector<std::thread> threads;
        threads.reserve(workers > 0 ? workers - 1 : 0);
        for (unsigned i = 1; i < workers; ++i) {
            threads.emplace_back(workerFn);
        }
        workerFn(); 
        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }

        for (int y = 0; y < height; ++y) {
            size_t rowOff = static_cast<size_t>(y) * static_cast<size_t>(width);
            for (int x = 0; x < width; ++x) {
                image->SetPixel(x, y, buffer[rowOff + static_cast<size_t>(x)]);
            }
        }
    }
};

} // namespace raytracer

#endif // RAYTRACER_RAYTRACER_HPP

