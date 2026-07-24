// include/camera.hpp

#pragma once

#include "vector3.hpp"
#include <cmath>
#include <algorithm>


class Camera {
public:
    Vector3 position{ 0.0f, 0.0f, 0.0f };

    // Both in radians. yaw = 0, pitch = 0 faces straight down +Z.
    float yaw = 0.0f;
    float pitch = 0.0f;

    float moveSpeed = 200.0f;          // world units per second
    float lookSensitivity = 0.0025f;   // radians per pixel of mouse motion

    static constexpr float kPitchLimit = 1.55334f; // ~89 degrees in radians

    // --- Orientation ---
    // These three vectors always stay unit length and mutually

    Vector3 forward() const {
        // Spherical-to-cartesian conversion: yaw sweeps around the Y
        // axis, pitch tilts up/down. cos(pitch) shrinks the horizontal
        // component as you look more steeply up or down.
        return {
            std::sin(yaw) * std::cos(pitch),
            std::sin(pitch),
            std::cos(yaw) * std::cos(pitch)
        };
    }

    Vector3 right() const {
        Vector3 f = forward();
        Vector3 r = { f.z, 0.0f, -f.x };
        float len = std::sqrt(r.x * r.x + r.z * r.z);
        if (len > 1e-6f) { r.x /= len; r.z /= len; }
        return r;
    }

    Vector3 up() const {
        Vector3 f = forward();
        Vector3 r = right();
        return {
            r.y * f.z - r.z * f.y,
            r.z * f.x - r.x * f.z,
            r.x * f.y - r.y * f.x
        };
    }

    // --- Input handling ---

    // dx/dy are raw mouse-motion deltas in pixels for this frame.
    void look(float dx, float dy) {
        yaw += dx * lookSensitivity;
        pitch -= dy * lookSensitivity; // moving the mouse up should look up, i.e. pitch increases
        pitch = std::clamp(pitch, -kPitchLimit, kPitchLimit);
    }

    // `amount` is a signed distance in world units (already scaled by
    // speed and delta time by the caller).
    void moveForward(float amount) {
        Vector3 f = forward();
        position.x += f.x * amount;
        position.y += f.y * amount;
        position.z += f.z * amount;
    }

    void moveRight(float amount) {
        Vector3 r = right();
        position.x += r.x * amount;
        position.y += r.y * amount;
        position.z += r.z * amount;
    }

    void moveUp(float amount) {
        position.y += amount;
    }

    // --- View transform ---

    // Converts a point from world space into camera space.
    Vector3 worldToCamera(const Vector3& worldPoint) const {
        // Step 1: translate so the camera is at the origin.
        Vector3 rel = {
            worldPoint.x - position.x,
            worldPoint.y - position.y,
            worldPoint.z - position.z
        };

        Vector3 f = forward();
        Vector3 r = right();
        Vector3 u = up();

        // Step 2: rotate. Projecting `rel` onto each basis vector (a dot
        // product) reads off its coordinate along that axis.
        return {
            rel.x * r.x + rel.y * r.y + rel.z * r.z,
            rel.x * u.x + rel.y * u.y + rel.z * u.z,
            rel.x * f.x + rel.y * f.y + rel.z * f.z
        };
    }
};