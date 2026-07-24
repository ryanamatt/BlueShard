// include/mesh.hpp

#pragma once

#include <SDL3/SDL.h>
#include "vector3.hpp"
#include "pixelBuffer.hpp"
#include "camera.hpp"
#include <vector>
#include <string>

struct Edge     { int v0, v1; };
struct Triangle { int v0, v1, v2; };

// A positioned, spinning triangle mesh - the reusable building block a
// Scene is made of.
class Mesh {
public:
    Vector3 position;
    Vector3 angularVelocity;

    // Advances rotation by angularVelocity and recomputes world-space
    // vertex positions. Call once per frame before render().
    void update();

    // Projects and rasterizes this mesh's visible faces + wireframe
    // into buffer, from camera's point of view.
    void render(PixelBuffer& buffer, const Camera& camera, const Vector3& lightDir) const;

    // --- Geometry factories ---
    static Mesh cube(float size);
    static Mesh plane(float width, float height,
                       uint8_t r = 200, uint8_t g = 200, uint8_t b = 200, uint8_t a = 255);
    static Mesh sphere(float radius, int segments = 16,
                        uint8_t r = 200, uint8_t g = 200, uint8_t b = 200, uint8_t a = 255);
    static Mesh cylinder(float radius, float height, int segments = 16,
                          uint8_t r = 200, uint8_t g = 200, uint8_t b = 200, uint8_t a = 255);

    // Loads a Wavefront .obj file. Supports "v" positions and "f" faces
    // (any of v, v/vt, v//vn, v/vt/vn per-vertex face tokens); n-gon
    // faces are triangulated as a fan. vt/vn/materials/groups are parsed
    // past but not yet used. Throws std::runtime_error if the file can't
    // be opened or contains no usable geometry.
    static Mesh fromOBJ(const std::string& path,
                         uint8_t r = 200, uint8_t g = 200, uint8_t b = 200, uint8_t a = 255);
    

private:
    std::vector<Vector3> localVertices;       // Vertex positions relative to (0,0,0), before rotation/translation
    std::vector<Vector3> transformedVertices; // Same vertices after the current rotation + position are applied
    std::vector<Edge> edges;
    std::vector<Triangle> triangles;
    std::vector<SDL_Color> faceColors;        // One color per face; assumes 2 triangles per face (triangles[i]/2 -> face index)

    Vector3 rotationAngles{ 0.0f, 0.0f, 0.0f };
};
