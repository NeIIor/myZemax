#ifndef RAYTRACER_RAY_HPP
#define RAYTRACER_RAY_HPP

#include "raytracer/vec3.hpp"

namespace raytracer {

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray() {}
    Ray(const Vec3& origin_, const Vec3& direction_) 
        : origin(origin_), direction(direction_.Normalized()) {}

    Vec3 At(float t) const {
        return origin + direction * t;
    }
};

} // namespace raytracer

#endif // RAYTRACER_RAY_HPP





