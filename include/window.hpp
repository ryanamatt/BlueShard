// include/window.hpp

#pragma once

#include <SDL3/SDL.h>
#include "pixelBuffer.hpp"
#include "scene.hpp"
#include <string>
#include <utility>

enum class Key { W, A, S, D, Space, LCtrl, LShift, Escape };

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isOpen() const { return isOpen_; }

    void pollEvents();

    float deltaTime() const { return deltaTime_; }
    float fps() const { return fps_; }

    bool keyDown(Key key) const;
    std::pair<float, float> mouseDelta() const { return { mouseDx_, mouseDy_ }; }

    // Frame drawing, in call order:
    void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255); // 1. clear the CPU buffer
    void renderScene(const Scene& scene);                         // 2. rasterize the scene into it, upload+draw it
    void drawText(const std::string& text, float x, float y);     // 3. (optional) draw text on top, e.g. an FPS counter
    void present();                                               // 4. flip to the screen

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    PixelBuffer buffer_;

    bool isOpen_ = true;

    Uint64 prevTicks_ = 0;
    Uint64 lastFpsTime_ = 0;
    Uint64 frameCount_ = 0;
    float fps_ = 0.0f;
    float deltaTime_ = 0.0f;

    float mouseDx_ = 0.0f;
    float mouseDy_ = 0.0f;

    void recreateTexture(int w, int h);
};
