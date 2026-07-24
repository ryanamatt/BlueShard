// include/scene.hpp

#pragma once

#include "camera.hpp"
#include "mesh.hpp"
#include "pixelBuffer.hpp"
#include <vector>
#include <memory>

// Everything needed to render one 3D view: a camera, and the objects in
// it.
class Scene {
public:
    Camera camera;

    std::shared_ptr<Mesh> addCube(Vector3 center, float size, Vector3 angularVelocity);

    void update();
    void render(PixelBuffer& buffer) const;

    const std::vector<std::shared_ptr<Mesh>>& objects() const { return objects_; }

private:
    std::vector<std::shared_ptr<Mesh>> objects_;
};
