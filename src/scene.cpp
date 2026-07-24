// src/scene.cpp

#include "scene.hpp"
#include <cmath>

std::shared_ptr<Mesh> Scene::addCube(Vector3 center, float size, Vector3 angularVelocity) {
    auto mesh = std::make_shared<Mesh>(Mesh::cube(size));
    mesh->position = center;
    mesh->angularVelocity = angularVelocity;
    objects_.push_back(mesh);
    return mesh;
}

std::shared_ptr<Mesh> Scene::addMesh(Mesh mesh, Vector3 position, Vector3 angularVelocity) {
    auto meshPtr = std::make_shared<Mesh>(std::move(mesh));
    meshPtr->position = position;
    meshPtr->angularVelocity = angularVelocity;
    objects_.push_back(meshPtr);
    return meshPtr;
}

void Scene::update() {
    for (auto& obj : objects_) {
        obj->update();
    }
}

void Scene::render(PixelBuffer& buffer) const {
    // Normalize once per frame rather than once per triangle.
    Vector3 lightDir = lightDirection;
    float len = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
    if (len > 1e-8f) {
        lightDir.x /= len;
        lightDir.y /= len;
        lightDir.z /= len;
    }

    for (const auto& obj : objects_) {
        obj->render(buffer, camera, lightDir);
    }
}
