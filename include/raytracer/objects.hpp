#ifndef RAYTRACER_OBJECTS_HPP
#define RAYTRACER_OBJECTS_HPP

#include "raytracer/object.hpp"
#include <cmath>
#include <algorithm>

namespace raytracer {

class Sphere : public Object {
public:
    float radius;

    Sphere(float radius_ = 1.0f, const std::string& name_ = "Sphere")
        : Object(name_), radius(radius_) {}

    HitResult Intersect(const Ray& ray) const override {
        HitResult result;
        Vec3 oc = ray.origin - position;
        float a = ray.direction.Dot(ray.direction);
        float b = 2.0f * oc.Dot(ray.direction);
        float c = oc.Dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) {
            return result;
        }

        float sqrt_d = sqrt(discriminant);
        float t1 = (-b - sqrt_d) / (2.0f * a);
        float t2 = (-b + sqrt_d) / (2.0f * a);

        float t = (t1 > 0.001f) ? t1 : ((t2 > 0.001f) ? t2 : -1.0f);
        if (t > 0.001f) {
            result.hit = true;
            result.t = t;
            result.point = ray.At(t);
            result.normal = (result.point - position).Normalized();
            result.object = this;
        }

        return result;
    }

    void GetBoundingBox(Vec3& min, Vec3& max) const override {
        min = position - Vec3(radius, radius, radius);
        max = position + Vec3(radius, radius, radius);
    }

    bool ContainsPoint(const Vec3& point) const override {
        Vec3 diff = point - position;
        return diff.LengthSquared() <= radius * radius;
    }
};

class Plane : public Object {
public:
    Vec3 normal;

    Plane(const Vec3& normal_ = Vec3(0, 1, 0), const std::string& name_ = "Plane")
        : Object(name_), normal(normal_.Normalized()) {}

    HitResult Intersect(const Ray& ray) const override {
        HitResult result;
        float denom = normal.Dot(ray.direction);
        
        if (fabs(denom) < 1e-6f) {
            return result;
        }

        Vec3 p0l0 = position - ray.origin;
        float t = p0l0.Dot(normal) / denom;

        if (t > 0.001f) {
            result.hit = true;
            result.t = t;
            result.point = ray.At(t);
            result.normal = normal;
            result.object = this;
        }

        return result;
    }

    void GetBoundingBox(Vec3& min, Vec3& max) const override {
        const float large = 1000.0f;
        min = Vec3(-large, -large, -large);
        max = Vec3(large, large, large);
    }

    bool ContainsPoint(const Vec3& point) const override {
        Vec3 diff = point - position;
        return fabs(diff.Dot(normal)) < 0.1f;
    }
};



class RectPlane : public Object {
public:
    Vec3 normal;
    float width;
    float height;

    RectPlane(float width_ = 2.0f,
              float height_ = 2.0f,
              const Vec3& normal_ = Vec3(0, 1, 0),
              const std::string& name_ = "RectPlane")
        : Object(name_), normal(normal_.Normalized()), width(width_), height(height_) {}

    HitResult Intersect(const Ray& ray) const override {
        HitResult result;
        float denom = normal.Dot(ray.direction);
        if (fabs(denom) < 1e-6f) {
            return result;
        }

        float t = (position - ray.origin).Dot(normal) / denom;
        if (t <= 0.001f) {
            return result;
        }

        Vec3 hitPoint = ray.At(t);

        
        Vec3 ref = (fabs(normal.y) < 0.95f) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
        Vec3 u = normal.Cross(ref).Normalized();
        Vec3 v = u.Cross(normal).Normalized();

        Vec3 d = hitPoint - position;
        float du = d.Dot(u);
        float dv = d.Dot(v);
        if (fabs(du) > width * 0.5f || fabs(dv) > height * 0.5f) {
            return result;
        }

        result.hit = true;
        result.t = t;
        result.point = hitPoint;
        result.normal = normal;
        result.object = this;
        return result;
    }

    void GetBoundingBox(Vec3& min, Vec3& max) const override {
        
        Vec3 ref = (fabs(normal.y) < 0.95f) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
        Vec3 u = normal.Cross(ref).Normalized();
        Vec3 v = u.Cross(normal).Normalized();

        Vec3 hu = u * (width * 0.5f);
        Vec3 hv = v * (height * 0.5f);
        Vec3 c0 = position + hu + hv;
        Vec3 c1 = position + hu - hv;
        Vec3 c2 = position - hu + hv;
        Vec3 c3 = position - hu - hv;

        min = Vec3(
            std::min(std::min(c0.x, c1.x), std::min(c2.x, c3.x)),
            std::min(std::min(c0.y, c1.y), std::min(c2.y, c3.y)),
            std::min(std::min(c0.z, c1.z), std::min(c2.z, c3.z))
        );
        max = Vec3(
            std::max(std::max(c0.x, c1.x), std::max(c2.x, c3.x)),
            std::max(std::max(c0.y, c1.y), std::max(c2.y, c3.y)),
            std::max(std::max(c0.z, c1.z), std::max(c2.z, c3.z))
        );

        
        Vec3 pad = normal * 0.01f;
        min -= Vec3(fabs(pad.x), fabs(pad.y), fabs(pad.z));
        max += Vec3(fabs(pad.x), fabs(pad.y), fabs(pad.z));
    }

    bool ContainsPoint(const Vec3& point) const override {
        Vec3 diff = point - position;
        if (fabs(diff.Dot(normal)) > 0.1f) return false;

        Vec3 ref = (fabs(normal.y) < 0.95f) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
        Vec3 u = normal.Cross(ref).Normalized();
        Vec3 v = u.Cross(normal).Normalized();

        float du = diff.Dot(u);
        float dv = diff.Dot(v);
        return fabs(du) <= width * 0.5f && fabs(dv) <= height * 0.5f;
    }
};

class Disk : public Object {
public:
    Vec3 normal;
    float radius;

    Disk(float radius_ = 1.0f, const Vec3& normal_ = Vec3(0, 1, 0), const std::string& name_ = "Disk")
        : Object(name_), normal(normal_.Normalized()), radius(radius_) {}

    HitResult Intersect(const Ray& ray) const override {
        HitResult result;
        float denom = normal.Dot(ray.direction);
        
        if (fabs(denom) < 1e-6f) {
            return result;
        }

        Vec3 p0l0 = position - ray.origin;
        float t = p0l0.Dot(normal) / denom;

        if (t > 0.001f) {
            Vec3 hitPoint = ray.At(t);
            Vec3 diff = hitPoint - position;
            float distSq = diff.Dot(diff);
            
            if (distSq <= radius * radius) {
                result.hit = true;
                result.t = t;
                result.point = hitPoint;
                result.normal = normal;
                result.object = this;
            }
        }

        return result;
    }

    void GetBoundingBox(Vec3& min, Vec3& max) const override {
        min = position - Vec3(radius, radius, radius);
        max = position + Vec3(radius, radius, radius);
    }

    bool ContainsPoint(const Vec3& point) const override {
        Vec3 diff = point - position;
        if (fabs(diff.Dot(normal)) > 0.1f) return false;
        return diff.Dot(diff) <= radius * radius;
    }
};

class Prism : public Object {
public:
    Vec3 size;

    Prism(const Vec3& size_ = Vec3(1, 1, 1), const std::string& name_ = "Prism")
        : Object(name_), size(size_) {}

    HitResult Intersect(const Ray& ray) const override {
        HitResult result;
        Vec3 invDir = Vec3(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);
        Vec3 min = position - size * 0.5f;
        Vec3 max = position + size * 0.5f;

        float tmin = (min.x - ray.origin.x) * invDir.x;
        float tmax = (max.x - ray.origin.x) * invDir.x;
        if (invDir.x < 0) std::swap(tmin, tmax);

        float tymin = (min.y - ray.origin.y) * invDir.y;
        float tymax = (max.y - ray.origin.y) * invDir.y;
        if (invDir.y < 0) std::swap(tymin, tymax);

        if (tmin > tymax || tymin > tmax) return result;

        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (min.z - ray.origin.z) * invDir.z;
        float tzmax = (max.z - ray.origin.z) * invDir.z;
        if (invDir.z < 0) std::swap(tzmin, tzmax);

        if (tmin > tzmax || tzmin > tmax) return result;

        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        if (tmin > 0.001f) {
            result.hit = true;
            result.t = tmin;
            result.point = ray.At(tmin);

            Vec3 center = (min + max) * 0.5f;
            Vec3 p = result.point - center;
            Vec3 absP(fabs(p.x), fabs(p.y), fabs(p.z));
            Vec3 halfSize = size * 0.5f;
            
            if (absP.x >= absP.y && absP.x >= absP.z) {
                result.normal = Vec3(p.x > 0 ? 1 : -1, 0, 0);
            } else if (absP.y >= absP.x && absP.y >= absP.z) {
                result.normal = Vec3(0, p.y > 0 ? 1 : -1, 0);
            } else {
                result.normal = Vec3(0, 0, p.z > 0 ? 1 : -1);
            }
            result.object = this;
        }

        return result;
    }

    void GetBoundingBox(Vec3& min, Vec3& max) const override {
        min = position - size * 0.5f;
        max = position + size * 0.5f;
    }

    bool ContainsPoint(const Vec3& point) const override {
        Vec3 halfSize = size * 0.5f;
        Vec3 diff = point - position;
        return fabs(diff.x) <= halfSize.x && 
               fabs(diff.y) <= halfSize.y && 
               fabs(diff.z) <= halfSize.z;
    }
};

class Pyramid : public Object {
public:
    float baseSize;
    float height;

    Pyramid(float baseSize_ = 1.0f, float height_ = 1.0f, const std::string& name_ = "Pyramid")
        : Object(name_), baseSize(baseSize_), height(height_) {}

    HitResult Intersect(const Ray& ray) const override {
        HitResult result;

        const float half = baseSize * 0.5f;
        const float baseY = position.y - height * 0.5f;
        const float apexY = position.y + height * 0.5f;

        const Vec3 P(position.x, apexY, position.z);
        const Vec3 A(position.x - half, baseY, position.z - half);
        const Vec3 B(position.x + half, baseY, position.z - half);
        const Vec3 C(position.x + half, baseY, position.z + half);
        const Vec3 D(position.x - half, baseY, position.z + half);

        const Vec3 inside(position.x, position.y, position.z);

        struct PlaneEq { Vec3 n; float d; }; 
        auto makePlane = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2) -> PlaneEq {
            Vec3 n = (p1 - p0).Cross(p2 - p0).Normalized();
            float d = -n.Dot(p0);
            float s = n.Dot(inside) + d;
            if (s > 0.0f) { n = -n; d = -d; }
            return {n, d};
        };

        PlaneEq planes[5] = {
            PlaneEq{Vec3(0, -1, 0), baseY},          
            makePlane(P, B, A),                      
            makePlane(P, C, B),                      
            makePlane(P, D, C),                      
            makePlane(P, A, D)                       
        };

        float tEnter = 0.001f;
        float tExit = 1e30f;
        Vec3 enterNormal(0, 1, 0);

        for (const auto& pl : planes) {
            float denom = pl.n.Dot(ray.direction);
            float dist = pl.n.Dot(ray.origin) + pl.d;
            if (fabs(denom) < 1e-6f) {
                if (dist > 0.0f) return result;
                continue;
            }
            float t = -dist / denom;
            if (denom > 0.0f) {
                tExit = std::min(tExit, t);
            } else {
                if (t > tEnter) {
                    tEnter = t;
                    enterNormal = pl.n;
                }
            }
            if (tEnter > tExit) return result;
        }

        if (tEnter > 0.001f) {
            result.hit = true;
            result.t = tEnter;
            result.point = ray.At(tEnter);
            result.normal = enterNormal;
            result.object = this;
        }
        return result;
    }

    void GetBoundingBox(Vec3& min, Vec3& max) const override {
        const float half = baseSize * 0.5f;
        const float baseY = position.y - height * 0.5f;
        const float apexY = position.y + height * 0.5f;
        min = Vec3(position.x - half, baseY, position.z - half);
        max = Vec3(position.x + half, apexY, position.z + half);
    }

    bool ContainsPoint(const Vec3& point) const override {
        const float half = baseSize * 0.5f;
        const float baseY = position.y - height * 0.5f;
        const float apexY = position.y + height * 0.5f;

        if (point.y < baseY || point.y > apexY) return false;

        const Vec3 P(position.x, apexY, position.z);
        const Vec3 A(position.x - half, baseY, position.z - half);
        const Vec3 B(position.x + half, baseY, position.z - half);
        const Vec3 C(position.x + half, baseY, position.z + half);
        const Vec3 D(position.x - half, baseY, position.z + half);
        const Vec3 inside(position.x, position.y, position.z);

        struct PlaneEq { Vec3 n; float d; };
        auto makePlane = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2) -> PlaneEq {
            Vec3 n = (p1 - p0).Cross(p2 - p0).Normalized();
            float d = -n.Dot(p0);
            float s = n.Dot(inside) + d;
            if (s > 0.0f) { n = -n; d = -d; }
            return {n, d};
        };

        PlaneEq planes[5] = {
            PlaneEq{Vec3(0, -1, 0), baseY},
            makePlane(P, B, A),
            makePlane(P, C, B),
            makePlane(P, D, C),
            makePlane(P, A, D)
        };

        for (const auto& pl : planes) {
            if (pl.n.Dot(point) + pl.d > 0.0f) return false;
        }
        return true;
    }
};

} // namespace raytracer

#endif // RAYTRACER_OBJECTS_HPP

