#ifndef RAYTRACER_VEC3_HPP
#define RAYTRACER_VEC3_HPP

#include <cmath>

namespace raytracer {

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float k) const { return Vec3(x * k, y * k, z * k); }
    Vec3 operator/(float k) const { return Vec3(x / k, y / k, z / k); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }

    Vec3& operator+=(const Vec3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vec3& operator-=(const Vec3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vec3& operator*=(float k) { x *= k; y *= k; z *= k; return *this; }
    Vec3& operator/=(float k) { x /= k; y /= k; z /= k; return *this; }

    float Dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vec3 Cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float Length() const { return sqrt(x * x + y * y + z * z); }
    float LengthSquared() const { return x * x + y * y + z * z; }
    Vec3 Normalized() const {
        float len = Length();
        if (len < 1e-6f) return Vec3(0, 0, 0);
        return *this / len;
    }
    void Normalize() {
        float len = Length();
        if (len > 1e-6f) *this /= len;
    }
};

inline Vec3 operator*(float k, const Vec3& v) {
    return v * k;
}

} // namespace raytracer

#endif // RAYTRACER_VEC3_HPP

