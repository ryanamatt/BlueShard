// src/mesh.cpp

#include "mesh.hpp"
#include "matrix.hpp"
#include <cmath>
#include <algorithm>

namespace {

    // Twice the signed area of triangle (a, b, p). Its sign tells us which
    // side of the line a->b the point p falls on - exactly what is needed
    // for a point-in-triangle test.
    inline float edgeFunction(float ax, float ay, float bx, float by, float px, float py) {
        return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    }

    // Fills a single 2D triangle by only visiting pixels inside its
    // bounding box, instead of scanning the whole framebuffer. Writes
    // straight into the CPU-side pixel buffer (see PixelBuffer).
    void rasterizeTriangle(PixelBuffer& buffer,
                            float x0, float y0,
                            float x1, float y1,
                            float x2, float y2,
                            SDL_Color color) {
        int screenW = buffer.width;
        int screenH = buffer.height;

        int minX = (int)std::floor(std::min({ x0, x1, x2 }));
        int maxX = (int)std::ceil (std::max({ x0, x1, x2 }));
        int minY = (int)std::floor(std::min({ y0, y1, y2 }));
        int maxY = (int)std::ceil (std::max({ y0, y1, y2 }));

        minX = std::max(minX, 0);
        minY = std::max(minY, 0);
        maxX = std::min(maxX, screenW - 1);
        maxY = std::min(maxY, screenH - 1);

        if (minX > maxX || minY > maxY) return;

        float area = edgeFunction(x0, y0, x1, y1, x2, y2);
        if (area == 0.0f) return;
        float sign = area < 0.0f ? -1.0f : 1.0f;

        for (int py = minY; py <= maxY; py++) {
            for (int px = minX; px <= maxX; px++) {
                float sx = px + 0.5f;
                float sy = py + 0.5f;

                float w0 = edgeFunction(x1, y1, x2, y2, sx, sy) * sign;
                float w1 = edgeFunction(x2, y2, x0, y0, sx, sy) * sign;
                float w2 = edgeFunction(x0, y0, x1, y1, sx, sy) * sign;

                if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                    buffer.setPixel(px, py, color.r, color.g, color.b, color.a);
                }
            }
        }
    }

} // namespace

Mesh Mesh::cube(float size) {
    Mesh mesh;
    float hs = size; // half-size

    mesh.localVertices = {
        { -hs, -hs, -hs }, {  hs, -hs, -hs }, {  hs,  hs, -hs }, { -hs,  hs, -hs },
        { -hs, -hs,  hs }, {  hs, -hs,  hs }, {  hs,  hs,  hs }, { -hs,  hs,  hs },
    };
    mesh.transformedVertices = mesh.localVertices;

    mesh.edges = {
        { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
        { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
        { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
    };

    mesh.triangles = {
        { 0, 1, 2 }, { 0, 2, 3 }, // back   (z = -hs)
        { 4, 6, 5 }, { 4, 7, 6 }, // front  (z = +hs)
        { 0, 3, 7 }, { 0, 7, 4 }, // left   (x = -hs)
        { 1, 5, 6 }, { 1, 6, 2 }, // right  (x = +hs)
        { 0, 4, 5 }, { 0, 5, 1 }, // top    (y = -hs)
        { 3, 2, 6 }, { 3, 6, 7 }, // bottom (y = +hs)
    };

    mesh.faceColors = {
        { 220,  60,  60, 255 }, // back   - red
        {  60, 220,  90, 255 }, // front  - green
        {  60, 120, 220, 255 }, // left   - blue
        { 230, 210,  60, 255 }, // right  - yellow
        { 200,  70, 210, 255 }, // top    - magenta
        {  60, 210, 210, 255 }, // bottom - cyan
    };

    return mesh;
}

void Mesh::update() {
    rotationAngles.x += angularVelocity.x;
    rotationAngles.y += angularVelocity.y;
    rotationAngles.z += angularVelocity.z;

    Matrix3 rotation = rotationZ(rotationAngles.z) * rotationY(rotationAngles.y) * rotationX(rotationAngles.x);

    for (size_t i = 0; i < localVertices.size(); i++) {
        Vector3 rotated = rotation * localVertices[i];
        transformedVertices[i] = {
            rotated.x + position.x,
            rotated.y + position.y,
            rotated.z + position.z
        };
    }
}

void Mesh::render(PixelBuffer& buffer, const Camera& camera) const {
    int width = buffer.width;
    int height = buffer.height;

    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float focalLength = 500.0f;

    // Move every vertex from world space into camera space once, up
    // front - see camera.hpp for what that means and why.
    std::vector<Vector3> camVerts(transformedVertices.size());
    for (size_t i = 0; i < transformedVertices.size(); i++) {
        camVerts[i] = camera.worldToCamera(transformedVertices[i]);
    }

    struct ProjVertex { float x, y; float viewZ; };
    std::vector<ProjVertex> proj(camVerts.size());

    for (size_t i = 0; i < camVerts.size(); i++) {
        float z = camVerts[i].z;
        if (z <= 0.1f) z = 0.1f;

        proj[i].x = (camVerts[i].x / z) * focalLength + centerX;
        proj[i].y = (camVerts[i].y / z) * focalLength + centerY;
        proj[i].viewZ = z;
    }

    // Back-face culling: keep only triangles whose normal points back
    // toward the camera (camera sits at the origin in camera space, so
    // a triangle's own v0 position is the "direction to the camera").
    std::vector<int> order;
    order.reserve(triangles.size());

    for (size_t i = 0; i < triangles.size(); i++) {
        const Triangle& tri = triangles[i];

        const Vector3& v0 = camVerts[tri.v0];
        const Vector3& v1 = camVerts[tri.v1];
        const Vector3& v2 = camVerts[tri.v2];

        float edge1x = v1.x - v0.x, edge1y = v1.y - v0.y, edge1z = v1.z - v0.z;
        float edge2x = v2.x - v0.x, edge2y = v2.y - v0.y, edge2z = v2.z - v0.z;

        float nx = edge1y * edge2z - edge1z * edge2y;
        float ny = edge1z * edge2x - edge1x * edge2z;
        float nz = edge1x * edge2y - edge1y * edge2x;

        float dot = nx * v0.x + ny * v0.y + nz * v0.z;
        if (dot < 0.0f) order.push_back((int)i);
    }

    // Painter's algorithm: farthest faces first, so nearer faces
    // correctly draw over them.
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        auto avgZ = [&](int t) {
            const Triangle& tri = triangles[t];
            return (proj[tri.v0].viewZ + proj[tri.v1].viewZ + proj[tri.v2].viewZ) / 3.0f;
        };
        return avgZ(a) > avgZ(b);
    });

    for (int idx : order) {
        const Triangle& tri = triangles[idx];
        const ProjVertex& p0 = proj[tri.v0];
        const ProjVertex& p1 = proj[tri.v1];
        const ProjVertex& p2 = proj[tri.v2];

        rasterizeTriangle(buffer, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, faceColors[idx / 2]);
    }

    // Outline the edges on top for a crisper silhouette.
    for (const Edge& e : edges) {
        const ProjVertex& a = proj[e.v0];
        const ProjVertex& b = proj[e.v1];
        buffer.drawLine((int)std::lround(a.x), (int)std::lround(a.y),
                         (int)std::lround(b.x), (int)std::lround(b.y),
                         20, 20, 20, 255);
    }
}
