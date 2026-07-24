// src/mesh.cpp

#include "mesh.hpp"
#include "matrix.hpp"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <set>
#include <utility>

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

    // Builds a wireframe edge list from a triangle list, deduping edges
    // shared by two triangles so shared edges aren't drawn twice. Used
    // by every factory except cube() (which hand-authors its edges to
    // control the exact silhouette).
    std::vector<Edge> buildEdgesFromTriangles(const std::vector<Triangle>& triangles) {
        std::set<std::pair<int, int>> unique;
        for (const auto& t : triangles) {
            int verts[3] = { t.v0, t.v1, t.v2 };
            for (int i = 0; i < 3; i++) {
                int a = verts[i];
                int b = verts[(i + 1) % 3];
                if (a > b) std::swap(a, b);
                unique.insert({ a, b });
            }
        }

        std::vector<Edge> edges;
        edges.reserve(unique.size());
        for (const auto& p : unique) edges.push_back({ p.first, p.second });
        return edges;
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

    // One entry per triangle (two triangles per face, same color), so
    // faceColors indexes 1:1 with `triangles` like every other factory.
    mesh.faceColors = {
        { 220,  60,  60, 255 }, { 220,  60,  60, 255 }, // back   - red
        {  60, 220,  90, 255 }, {  60, 220,  90, 255 }, // front  - green
        {  60, 120, 220, 255 }, {  60, 120, 220, 255 }, // left   - blue
        { 230, 210,  60, 255 }, { 230, 210,  60, 255 }, // right  - yellow
        { 200,  70, 210, 255 }, { 200,  70, 210, 255 }, // top    - magenta
        {  60, 210, 210, 255 }, {  60, 210, 210, 255 }, // bottom - cyan
    };

    return mesh;
}

Mesh Mesh::plane(float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Mesh mesh;
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    // A single quad in the XY plane (facing +Z, i.e. facing the default
    // camera) - handy as a flat canvas for icons/widget backgrounds.
    mesh.localVertices = {
        { -hw, -hh, 0.0f }, {  hw, -hh, 0.0f }, {  hw,  hh, 0.0f }, { -hw,  hh, 0.0f },
    };
    mesh.transformedVertices = mesh.localVertices;
    mesh.triangles = { { 0, 1, 2 }, { 0, 2, 3 } };
    mesh.edges = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } };
    mesh.faceColors.assign(mesh.triangles.size(), SDL_Color{ r, g, b, a });

    return mesh;
}

Mesh Mesh::sphere(float radius, int segments, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Mesh mesh;
    constexpr float kPi = 3.14159265358979323846f;

    int rings = std::max(segments / 2, 2);
    int sectors = std::max(segments, 3);
    int vertsPerRing = sectors + 1;

    // Standard UV sphere: ring by ring from the top pole (ring 0) to the
    // bottom pole (ring == rings), each ring holding `sectors + 1`
    // vertices (the +1 duplicates the seam vertex so texture/seam
    // wrapping stays simple - unused today but cheap to keep).
    for (int ring = 0; ring <= rings; ring++) {
        float v = (float)ring / (float)rings;
        float phi = v * kPi;
        float y = std::cos(phi) * radius;
        float ringRadius = std::sin(phi) * radius;

        for (int sec = 0; sec <= sectors; sec++) {
            float u = (float)sec / (float)sectors;
            float theta = u * 2.0f * kPi;
            float x = std::cos(theta) * ringRadius;
            float z = std::sin(theta) * ringRadius;
            mesh.localVertices.push_back({ x, y, z });
        }
    }

    for (int ring = 0; ring < rings; ring++) {
        for (int sec = 0; sec < sectors; sec++) {
            int i0 = ring * vertsPerRing + sec;
            int i1 = i0 + 1;
            int i2 = i0 + vertsPerRing;
            int i3 = i2 + 1;

            // Degenerate triangles at the poles (i0 == i2's ring) are
            // skipped since the top/bottom ring collapses to a point.
            if (ring > 0)          mesh.triangles.push_back({ i0, i2, i1 });
            if (ring < rings - 1)  mesh.triangles.push_back({ i1, i2, i3 });
        }
    }

    mesh.transformedVertices = mesh.localVertices;
    mesh.edges = buildEdgesFromTriangles(mesh.triangles);
    mesh.faceColors.assign(mesh.triangles.size(), SDL_Color{ r, g, b, a });

    return mesh;
}

Mesh Mesh::cylinder(float radius, float height, int segments, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Mesh mesh;
    constexpr float kPi = 3.14159265358979323846f;

    int sectors = std::max(segments, 3);
    float halfHeight = height * 0.5f;

    std::vector<int> topRing, bottomRing;
    topRing.reserve(sectors);
    bottomRing.reserve(sectors);

    for (int sec = 0; sec < sectors; sec++) {
        float theta = (float)sec / (float)sectors * 2.0f * kPi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        mesh.localVertices.push_back({ x, halfHeight, z });
        topRing.push_back((int)mesh.localVertices.size() - 1);
    }
    for (int sec = 0; sec < sectors; sec++) {
        float theta = (float)sec / (float)sectors * 2.0f * kPi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        mesh.localVertices.push_back({ x, -halfHeight, z });
        bottomRing.push_back((int)mesh.localVertices.size() - 1);
    }

    // Side wall, one quad (two triangles) per sector.
    for (int sec = 0; sec < sectors; sec++) {
        int next = (sec + 1) % sectors;
        int t0 = topRing[sec],    t1 = topRing[next];
        int b0 = bottomRing[sec], b1 = bottomRing[next];
        mesh.triangles.push_back({ t0, b0, b1 });
        mesh.triangles.push_back({ t0, b1, t1 });
    }

    // Flat end caps, fanned from a center vertex.
    mesh.localVertices.push_back({ 0.0f, halfHeight, 0.0f });
    int topCenter = (int)mesh.localVertices.size() - 1;
    mesh.localVertices.push_back({ 0.0f, -halfHeight, 0.0f });
    int bottomCenter = (int)mesh.localVertices.size() - 1;

    for (int sec = 0; sec < sectors; sec++) {
        int next = (sec + 1) % sectors;
        mesh.triangles.push_back({ topCenter, topRing[next], topRing[sec] });
        mesh.triangles.push_back({ bottomCenter, bottomRing[sec], bottomRing[next] });
    }

    mesh.transformedVertices = mesh.localVertices;
    mesh.edges = buildEdgesFromTriangles(mesh.triangles);
    mesh.faceColors.assign(mesh.triangles.size(), SDL_Color{ r, g, b, a });

    return mesh;
}

Mesh Mesh::fromOBJ(const std::string& path, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Mesh mesh;

    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open OBJ file: " + path);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            mesh.localVertices.push_back({ x, y, z });
        } else if (tag == "f") {
            // Each face token is "v", "v/vt", "v//vn", or "v/vt/vn" - we
            // only need the vertex index. vt/vn are parsed past, not used.
            std::vector<int> faceVerts;
            std::string token;
            while (ss >> token) {
                size_t slashPos = token.find('/');
                std::string idxStr = (slashPos == std::string::npos) ? token : token.substr(0, slashPos);
                int idx = std::stoi(idxStr);
                // OBJ indices are 1-based; negative indices count back
                // from the current end of the vertex list.
                if (idx < 0) idx = (int)mesh.localVertices.size() + idx + 1;
                faceVerts.push_back(idx - 1);
            }
            // Fan-triangulate n-gons (works for the common convex case;
            // OBJ faces are typically triangles or quads in practice).
            for (size_t i = 1; i + 1 < faceVerts.size(); i++) {
                mesh.triangles.push_back({ faceVerts[0], faceVerts[i], faceVerts[i + 1] });
            }
        }
        // Other tags (vt, vn, mtllib, usemtl, o, g, s, comments...) are
        // intentionally ignored for now.
    }

    if (mesh.localVertices.empty() || mesh.triangles.empty()) {
        throw std::runtime_error("OBJ file had no usable geometry: " + path);
    }

    // Re-center on the mesh's own bounding-box center so imported meshes
    // rotate about themselves, same as the built-in primitives.
    Vector3 minB = mesh.localVertices[0];
    Vector3 maxB = mesh.localVertices[0];
    for (const auto& v : mesh.localVertices) {
        minB.x = std::min(minB.x, v.x); maxB.x = std::max(maxB.x, v.x);
        minB.y = std::min(minB.y, v.y); maxB.y = std::max(maxB.y, v.y);
        minB.z = std::min(minB.z, v.z); maxB.z = std::max(maxB.z, v.z);
    }
    Vector3 center = {
        (minB.x + maxB.x) * 0.5f,
        (minB.y + maxB.y) * 0.5f,
        (minB.z + maxB.z) * 0.5f
    };
    for (auto& v : mesh.localVertices) {
        v.x -= center.x;
        v.y -= center.y;
        v.z -= center.z;
    }

    mesh.transformedVertices = mesh.localVertices;
    mesh.edges = buildEdgesFromTriangles(mesh.triangles);
    mesh.faceColors.assign(mesh.triangles.size(), SDL_Color{ r, g, b, a });

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

void Mesh::render(PixelBuffer& buffer, const Camera& camera, const Vector3& lightDir) const {
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

    // Below the floor, a triangle facing straight away from the light
    // still reads as its base color at 25% brightness rather than black -
    // avoids fully unlit faces looking like rendering artifacts.
    constexpr float kAmbientFloor = 0.25f;

    for (int idx : order) {
        const Triangle& tri = triangles[idx];
        const ProjVertex& p0 = proj[tri.v0];
        const ProjVertex& p1 = proj[tri.v1];
        const ProjVertex& p2 = proj[tri.v2];

        // World-space face normal (separate from the camera-space one
        // used for culling above), so shading doesn't depend on the
        // camera's orientation - only on the mesh's rotation.
        const Vector3& w0 = transformedVertices[tri.v0];
        const Vector3& w1 = transformedVertices[tri.v1];
        const Vector3& w2 = transformedVertices[tri.v2];

        float e1x = w1.x - w0.x, e1y = w1.y - w0.y, e1z = w1.z - w0.z;
        float e2x = w2.x - w0.x, e2y = w2.y - w0.y, e2z = w2.z - w0.z;

        float wnx = e1y * e2z - e1z * e2y;
        float wny = e1z * e2x - e1x * e2z;
        float wnz = e1x * e2y - e1y * e2x;
        float wlen = std::sqrt(wnx * wnx + wny * wny + wnz * wnz);

        float lightAmount = 1.0f;
        if (wlen > 1e-8f) {
            wnx /= wlen; wny /= wlen; wnz /= wlen;
            float d = wnx * lightDir.x + wny * lightDir.y + wnz * lightDir.z;
            lightAmount = std::max(kAmbientFloor, d);
        }

        SDL_Color base = faceColors[idx];
        SDL_Color shaded {
            (uint8_t)std::clamp(base.r * lightAmount, 0.0f, 255.0f),
            (uint8_t)std::clamp(base.g * lightAmount, 0.0f, 255.0f),
            (uint8_t)std::clamp(base.b * lightAmount, 0.0f, 255.0f),
            base.a
        };

        rasterizeTriangle(buffer, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, shaded);
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
