#ifndef RAYTRACER_SCENE_HPP
#define RAYTRACER_SCENE_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include "raytracer/object.hpp"
#include "raytracer/vec3.hpp"

namespace raytracer {

class Scene {
public:
    std::vector<std::unique_ptr<Object>> objects;

    void AddObject(std::unique_ptr<Object> obj) {
        objects.push_back(std::move(obj));
    }

    void RemoveObject(Object* obj) {
        objects.erase(
            std::remove_if(objects.begin(), objects.end(),
                [obj](const std::unique_ptr<Object>& o) { return o.get() == obj; }),
            objects.end()
        );
    }

    Object* FindObjectByName(const std::string& name) {
        for (auto& obj : objects) {
            if (obj->name == name) {
                return obj.get();
            }
        }
        return nullptr;
    }

    Object* FindObjectAtPoint(const Vec3& point) {
        for (auto& obj : objects) {
            if (obj->ContainsPoint(point)) {
                return obj.get();
            }
        }
        return nullptr;
    }
};

} // namespace raytracer

#endif // RAYTRACER_SCENE_HPP

