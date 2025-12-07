#ifndef RAYTRACER_CAMERA_HPP
#define RAYTRACER_CAMERA_HPP

#include <cmath>
#include <algorithm>
#include "raytracer/vec3.hpp"
#include "raytracer/ray.hpp"

namespace raytracer {

class Camera {
public:
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    float moveSpeed = 5.0f;
    float rotationSpeed = 2.0f;
    float maxSpeed = 20.0f;
    float acceleration = 10.0f;

    float currentMoveSpeed = 0.0f;
    float currentRotationSpeed = 0.0f;

    Camera(const Vec3& pos = Vec3(0, 0, 5), 
           const Vec3& target_ = Vec3(0, 0, 0),
           const Vec3& up_ = Vec3(0, 1, 0),
           float fov_ = 60.0f)
        : position(pos), target(target_), up(up_), fov(fov_),
          aspectRatio(16.0f / 9.0f), nearPlane(0.1f), farPlane(100.0f) {}

    Vec3 GetForward() const {
        return (target - position).Normalized();
    }

    Vec3 GetRight() const {
        Vec3 forward = GetForward();
        return forward.Cross(up).Normalized();
    }

    Vec3 GetUp() const {
        Vec3 forward = GetForward();
        Vec3 right = GetRight();
        return right.Cross(forward).Normalized();
    }

    Ray GetRay(float screenX, float screenY, float screenWidth, float screenHeight) const {
        float ndcX = (2.0f * screenX / screenWidth) - 1.0f;
        float ndcY = 1.0f - (2.0f * screenY / screenHeight);
        
        float tanHalfFov = tan(fov * 3.14159f / 180.0f * 0.5f);
        float viewX = ndcX * aspectRatio * tanHalfFov;
        float viewY = ndcY * tanHalfFov;

        Vec3 forward = GetForward();
        Vec3 right = GetRight();
        Vec3 up = GetUp();

        Vec3 direction = (forward + right * viewX + up * viewY).Normalized();
        return Ray(position, direction);
    }

    void MoveForward(float deltaTime) {
        Vec3 forward = GetForward();
        position += forward * currentMoveSpeed * deltaTime;
        target += forward * currentMoveSpeed * deltaTime;
    }

    void MoveBackward(float deltaTime) {
        Vec3 forward = GetForward();
        position -= forward * currentMoveSpeed * deltaTime;
        target -= forward * currentMoveSpeed * deltaTime;
    }

    void MoveRight(float deltaTime) {
        Vec3 right = GetRight();
        position += right * currentMoveSpeed * deltaTime;
        target += right * currentMoveSpeed * deltaTime;
    }

    void MoveLeft(float deltaTime) {
        Vec3 right = GetRight();
        position -= right * currentMoveSpeed * deltaTime;
        target -= right * currentMoveSpeed * deltaTime;
    }

    void MoveUp(float deltaTime) {
        Vec3 up = GetUp();
        position += up * currentMoveSpeed * deltaTime;
        target += up * currentMoveSpeed * deltaTime;
    }

    void MoveDown(float deltaTime) {
        Vec3 up = GetUp();
        position -= up * currentMoveSpeed * deltaTime;
        target -= up * currentMoveSpeed * deltaTime;
    }

    void RotateYaw(float angle) {
        Vec3 forward = GetForward();
        Vec3 right = GetRight();
        Vec3 up = GetUp();
        
        float cosA = cos(angle);
        float sinA = sin(angle);
        Vec3 newForward = forward * cosA + right * sinA;
        target = position + newForward;
    }

    void RotatePitch(float angle) {
        Vec3 forward = GetForward();
        Vec3 up = GetUp();
        
        Vec3 right = GetRight();
        float cosA = cos(angle);
        float sinA = sin(angle);
        Vec3 newForward = forward * cosA + up * sinA;
        
        float pitch = asin(newForward.y);
        if (pitch > 1.5f || pitch < -1.5f) return;
        
        target = position + newForward;
    }

    void UpdateSpeed(float deltaTime, bool isMoving) {
        if (isMoving) {
            currentMoveSpeed = std::min(currentMoveSpeed + acceleration * deltaTime, maxSpeed);
        } else {
            currentMoveSpeed = std::max(0.0f, currentMoveSpeed - acceleration * deltaTime * 2.0f);
        }
    }

    void UpdateRotationSpeed(float deltaTime, bool isRotating) {
        if (isRotating) {
            currentRotationSpeed = std::min(currentRotationSpeed + acceleration * deltaTime, maxSpeed);
        } else {
            currentRotationSpeed = std::max(0.0f, currentRotationSpeed - acceleration * deltaTime * 2.0f);
        }
    }
};

} // namespace raytracer

#endif // RAYTRACER_CAMERA_HPP

