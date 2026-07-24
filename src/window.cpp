// src/window.cpp

#include "window.hpp"
#include <stdexcept>

Window::Window(const std::string& title, int width, int height) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("SDL could not initialize: ") + SDL_GetError());
    }

    window_ = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_RESIZABLE);
    if (!window_) {
        SDL_Quit();
        throw std::runtime_error(std::string("Window could not be created: ") + SDL_GetError());
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::string err = SDL_GetError();
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error("Renderer could not be created: " + err);
    }

    SDL_SetRenderVSync(renderer_, 1);

    buffer_.resize(width, height);
    recreateTexture(width, height);

    prevTicks_ = SDL_GetTicks();
    lastFpsTime_ = prevTicks_;
}

Window::~Window() {
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Window::recreateTexture(int w, int h) {
    if (texture_) SDL_DestroyTexture(texture_);

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!texture_) {
        throw std::runtime_error(std::string("Texture could not be created: ") + SDL_GetError());
    }
    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
}

void Window::pollEvents() {
    // Reset per-frame accumulators before draining new events.
    mouseDx_ = 0.0f;
    mouseDy_ = 0.0f;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            isOpen_ = false;
        }

        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            int newW = event.window.data1;
            int newH = event.window.data2;
            buffer_.resize(newW, newH);
            recreateTexture(newW, newH);
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            mouseDx_ += event.motion.xrel;
            mouseDy_ += event.motion.yrel;
        }
    }

    Uint64 now = SDL_GetTicks();
    deltaTime_ = (float)(now - prevTicks_) / 1000.0f;
    prevTicks_ = now;

    frameCount_++;
    if (now - lastFpsTime_ >= 500) {
        fps_ = (float)frameCount_ * 1000.0f / (float)(now - lastFpsTime_);
        lastFpsTime_ = now;
        frameCount_ = 0;
    }
}

bool Window::keyDown(Key key) const {
    SDL_Scancode scancode;
    switch (key) {
        case Key::W:      scancode = SDL_SCANCODE_W;      break;
        case Key::A:      scancode = SDL_SCANCODE_A;      break;
        case Key::S:      scancode = SDL_SCANCODE_S;      break;
        case Key::D:      scancode = SDL_SCANCODE_D;      break;
        case Key::Space:  scancode = SDL_SCANCODE_SPACE;  break;
        case Key::LCtrl:  scancode = SDL_SCANCODE_LCTRL;  break;
        case Key::LShift: scancode = SDL_SCANCODE_LSHIFT; break;
        case Key::Escape: scancode = SDL_SCANCODE_ESCAPE; break;
        default:          return false;
    }

    const bool* keys = SDL_GetKeyboardState(nullptr);
    return keys[scancode];
}

void Window::clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    buffer_.clear(r, g, b, a);
}

void Window::renderScene(const Scene& scene) {
    scene.render(buffer_);

    // One upload of the whole CPU buffer to the GPU, then one draw call
    // for the textured quad that fills the window - see pixelBuffer.hpp
    // for why this beats drawing pixel-by-pixel through the renderer.
    SDL_UpdateTexture(texture_, nullptr, buffer_.pixels.data(),
                       buffer_.width * (int)sizeof(uint32_t));
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
}

void Window::drawText(const std::string& text, float x, float y) {
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer_, x, y, text.c_str());
}

void Window::present() {
    SDL_RenderPresent(renderer_);
}
