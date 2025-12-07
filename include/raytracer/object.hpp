#ifndef RAYTRACER_OBJECT_HPP
#define RAYTRACER_OBJECT_HPP

#include <string>
#include "raytracer/ray.hpp"
#include "raytracer/vec3.hpp"
#include "dr4/math/color.hpp"

namespace raytracer {

struct HitResult {
    bool hit = false;
    float t = 0.0f;
    Vec3 point;
    Vec3 normal;
    const class Object* object = nullptr;
};

class Object {
public:
    std::string name;
    Vec3 position;
    dr4::Color color;
    float refractiveIndex = 1.0f;
    float reflectivity = 0.0f;
    bool isLightSource = false;

    Object(const std::string& name_ = "Object")
        : name(name_), position(0, 0, 0), color(255, 255, 255) {}

    virtual ~Object() = default;

    virtual HitResult Intersect(const Ray& ray) const = 0;
    virtual void GetBoundingBox(Vec3& min, Vec3& max) const = 0;
    virtual bool ContainsPoint(const Vec3& point) const = 0;
};

} // namespace raytracer

#endif // RAYTRACER_OBJECT_HPP



