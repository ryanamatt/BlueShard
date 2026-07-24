// src/scene.cpp

#include "scene.hpp"

std::shared_ptr<Mesh> Scene::addCube(Vector3 center, float size, Vector3 angularVelocity) {
    auto mesh = std::make_shared<Mesh>(Mesh::cube(size));
    mesh->position = center;
    mesh->angularVelocity = angularVelocity;
    objects_.push_back(mesh);
    return mesh;
}

void Scene::update() {
    for (auto& obj : objects_) {
        obj->update();
    }
}

void Scene::render(PixelBuffer& buffer) const {
    for (const auto& obj : objects_) {
        obj->render(buffer, camera);
    }
}
