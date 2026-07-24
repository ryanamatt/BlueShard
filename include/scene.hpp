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

    // Unit-ish vector pointing *toward* the light source in world space
    // (doesn't need to be pre-normalized; render() normalizes a copy
    // each frame). Default is an overhead-front light.
    Vector3 lightDirection{ 0.3f, -0.8f, 0.4f };

    std::shared_ptr<Mesh> addCube(Vector3 center, float size, Vector3 angularVelocity);

    // Takes ownership of an already-built mesh (e.g. from Mesh::fromOBJ,
    // Mesh::sphere, Mesh::cylinder, Mesh::plane) and places it in the
    // scene. Prefer this over addCube for any non-cube geometry.
    std::shared_ptr<Mesh> addMesh(Mesh mesh, Vector3 position, Vector3 angularVelocity);

    void update();
    void render(PixelBuffer& buffer) const;

    const std::vector<std::shared_ptr<Mesh>>& objects() const { return objects_; }

private:
    std::vector<std::shared_ptr<Mesh>> objects_;
};