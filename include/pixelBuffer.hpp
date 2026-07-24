// include pixelBuffer.hpp

#pragma once
 
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <algorithm>

class PixelBuffer {
public:
    int width = 0;
    int height = 0;

    // Row-major, top-left origin. One uint32_t per pixel, packed RGBA8888
    // (matches SDL_PIXELFORMAT_RGBA8888 so it can be uploaded as-is).
    std::vector<uint32_t> pixels;

    void resize(int w, int h) {
        width = w;
        height = h;
        pixels.assign(static_cast<size_t>(w) * static_cast<size_t>(h), 0);
    }

    inline void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        std::fill(pixels.begin(), pixels.end(), pack(r, g, b, a));
    }

    
    // Bounds-checked single pixel write.
    inline void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        if (static_cast<unsigned>(x) >= static_cast<unsigned>(width) ||
            static_cast<unsigned>(y) >= static_cast<unsigned>(height)) {
            return;
        }
        pixels[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] =
            pack(r, g, b, a);
    }
 
    // Bresenham's integer line algorithm, used to draw the wireframe
    // edges directly into the buffer.
    void drawLine(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        int dx = std::abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
        int err = dx + dy;
 
        while (true) {
            setPixel(x0, y0, r, g, b, a);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
 
    static inline uint32_t pack(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return (static_cast<uint32_t>(r) << 24) |
               (static_cast<uint32_t>(g) << 16) |
               (static_cast<uint32_t>(b) << 8)  |
                static_cast<uint32_t>(a);
    }
};