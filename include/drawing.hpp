#pragma once
#include "utils.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

struct WindowState {
    bool is_texture;
    int texture_id;
    int width, height, offsetx, offsety;
};

class Window {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Color curcolor{255, 255, 255, 255};
    std::vector<SDL_Texture *> textures;
    std::vector<TTF_Font *> fonts;
    int width, height;
    bool rendering_on_texture = false;
    int current_texture = 0;
    std::unordered_map<SDL_Keycode, bool> keymap, prev_keymap;
    bool rightClick = false;
    bool leftClick = false;
    bool running = true;

  public:
    int frames = 0;
    int offsetx = 0, offsety = 0;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    Window(const std::string &title, int width, int height) : width(width), height(height) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
            return;
        }
        // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
            return;
        }
        if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
            std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
            return;
        }
        window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                  SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
            return;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
            return;
        }
    }
    void setCursorVisible(bool visible) { SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE); }
    void poll() {
        prev_keymap = keymap;
        static SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT)
                running = 0;
            if (e.type == SDL_KEYDOWN) {
                keymap[e.key.keysym.sym] = 1;
            }
            if (e.type == SDL_KEYUP) {
                keymap[e.key.keysym.sym] = 0;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    leftClick = 1;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    rightClick = 1;
                }
            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    leftClick = 0;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    rightClick = 0;
                }
            }
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    width = e.window.data1;
                    height = e.window.data2;
                }
            }
        }
    }

    bool isRunning() { return running; }
    bool isPressed(SDL_Keycode k) { return keymap[k]; }
    bool isJustPressed(SDL_Keycode k) { return keymap[k] && !prev_keymap[k]; }
    bool isJustReleased(SDL_Keycode k) { return !keymap[k] && prev_keymap[k]; }
    bool isLeftClicked() { return leftClick; }
    bool isRightClicked() { return rightClick; }
    vec2d mousePosition() {
        static SDL_Point mouse;
        SDL_GetMouseState(&mouse.x, &mouse.y);
        return vec2d(mouse.x, mouse.y);
    }
    WindowState getState() { return {rendering_on_texture, current_texture, width, height, offsetx, offsety}; }
    void setState(WindowState w) {
        rendering_on_texture = w.is_texture;
        current_texture = w.texture_id;
        width = w.width;
        height = w.height;
        offsetx = w.offsetx;
        offsety = w.offsety;
        if (rendering_on_texture && current_texture >= 0 && current_texture < textures.size() &&
            textures[current_texture]) {
            SDL_SetRenderTarget(renderer, textures[current_texture]);
        } else {
            rendering_on_texture = false;
            SDL_SetRenderTarget(renderer, NULL);
        }
    }
    void setState(int i) {
        if (i < 0 || i >= textures.size() || !textures[i]) {
            std::cerr << "Error: Texture ID " << i << " out of bounds." << std::endl;
            return;
        }
        rendering_on_texture = true;
        current_texture = i;
        SDL_SetRenderTarget(renderer, textures[i]);
        SDL_QueryTexture(textures[i], NULL, NULL, &width, &height);
        offsetx = 0;
        offsety = 0;
    }
    SDL_Rect windowDimensions() { return {offsetx, offsety, width, height}; }

    ~Window() {
        for (int i = 0; i < textures.size(); i++) {
            if (textures[i]) {
                SDL_DestroyTexture(textures[i]);
                textures[i] = nullptr;
            }
        }
        for (int i = 0; i < fonts.size(); i++) {
            if (fonts[i]) {
                TTF_CloseFont(fonts[i]);
                fonts[i] = nullptr;
            }
        }
        if (renderer)
            SDL_DestroyRenderer(renderer);
        if (window)
            SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
    }

    int createTargetTexture(int w, int h) {
        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        if (!texture) {
            std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
            return -1;
        }
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        textures.push_back(texture);
        return textures.size() - 1;
    }
    void setRenderTarget(int i) {
        if (i >= 0 && i < textures.size() && textures[i]) {
            if (SDL_SetRenderTarget(renderer, textures[i]) != 0) {
                std::cerr << "SDL_SetRenderTarget Error: " << SDL_GetError() << std::endl;
            }
        }
    }
    void resetRenderTarget() { SDL_SetRenderTarget(renderer, NULL); }
    void QueryTexture(int i, int &w, int &h) {
        if (i >= 0 && i < textures.size() && textures[i]) {
            SDL_QueryTexture(textures[i], NULL, NULL, &w, &h);
        }
    }
    void drawTexture(int i, SDL_Rect img, SDL_Rect win) {
        if (i < 0 || i >= textures.size() || !textures[i])
            return;
        SDL_Texture *texture = textures[i];
        win.x += offsetx;
        win.y += offsety;
        SDL_RenderCopy(renderer, texture, &img, &win);
    }
    void drawTexture(int i, SDL_Rect win) {
        if (i < 0 || i >= textures.size() || !textures[i])
            return;
        SDL_Texture *texture = textures[i];
        win.x += offsetx;
        win.y += offsety;
        SDL_Rect img{0, 0, 0, 0};
        SDL_QueryTexture(texture, NULL, NULL, &img.w, &img.h);
        SDL_RenderCopy(renderer, texture, &img, &win);
    }
    void drawScrollingTexture(int i, int scroll) {
        if (i < 0 || i >= textures.size() || !textures[i])
            return;
        if (scroll == 0)
            scroll = 1;
        SDL_Texture *texture = textures[i];
        scroll = -offsetx / scroll;
        int w, h;
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        scroll = (scroll % w + w) % w;
        SDL_Rect win;
        SDL_Rect img;
        if (w - scroll > h * width / height) {
            win = {0, 0, width, height};
            img = {scroll, 0, h * width / height, h};
        } else {
            int a = (w - scroll) * height / h;
            int b = h * width / height;
            win = {a, 0, width - a, height};
            img = {0, 0, b - w + scroll, h};
            SDL_RenderCopy(renderer, texture, &img, &win);
            win = {0, 0, a, height};
            img = {scroll, 0, w - scroll, h};
        }
        SDL_RenderCopy(renderer, texture, &img, &win);
    }
    void animate(int i, SDL_Rect loc, int fr) {
        if (i < 0 || i >= textures.size() || !textures[i] || fr <= 0)
            return;
        int f = (frames / 20) % fr;
        SDL_Texture *texture = textures[i];
        SDL_Rect img{0, 0, 0, 0};
        SDL_QueryTexture(texture, NULL, NULL, &img.w, &img.h);
        drawTexture(i, {f * img.w / fr, 0, img.w / fr, img.h}, loc);
    }
    void drawFrame(int i, SDL_Rect loc, int frx, int fry) {
        if (i < 0 || i >= textures.size() || !textures[i] || frx <= 0 || fry <= 0)
            return;
        int total = frx * fry;
        int f = (frames / 20) % total;
        int x = f % frx;
        int y = f / frx;
        SDL_Texture *texture = textures[i];
        SDL_Rect img{0, 0, 0, 0};
        SDL_QueryTexture(texture, NULL, NULL, &img.w, &img.h);
        drawTexture(i, {x * img.w / frx, y * img.h / fry, img.w / frx, img.h / fry}, loc);
    }
    void drawFrame(int i, int f, SDL_Rect loc, int frx, int fry) {
        if (i < 0 || i >= textures.size() || !textures[i] || frx <= 0 || fry <= 0)
            return;
        int x = f % frx;
        int y = f / frx;
        SDL_Texture *texture = textures[i];
        SDL_Rect img{0, 0, 0, 0};
        SDL_QueryTexture(texture, NULL, NULL, &img.w, &img.h);
        drawTexture(i, {x * img.w / frx, y * img.h / fry, img.w / frx, img.h / fry}, loc);
    }
    void drawBg(int i) {
        if (i < 0 || i >= textures.size() || !textures[i])
            return;
        SDL_Texture *texture = textures[i];
        SDL_Rect img{0, 0, 0, 0};
        SDL_Rect win{0, 0, width, height};
        SDL_QueryTexture(texture, NULL, NULL, &img.w, &img.h);
        SDL_RenderCopy(renderer, texture, &img, &win);
    }
    int Width() { return width; }
    int Height() { return height; }
    void setOffsets(int x, int y) {
        offsetx = x;
        offsety = y;
    }
    void setNegativeOffsets(int x, int y) {
        offsetx = -x;
        offsety = -y;
    }
    void setColor(SDL_Color c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        curcolor = c;
    }
    void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        curcolor = {r, g, b, a};
    }
    void drawPoint(int x, int y) { SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety); }
    void drawLine(int x1, int y1, int x2, int y2) {
        aalineRGBA(renderer, x1 + offsetx, y1 + offsety, x2 + offsetx, y2 + offsety, curcolor.r, curcolor.g, curcolor.b,
                   curcolor.a);
    }
    void drawThickLine(int x1, int y1, int x2, int y2, int size) {
        thickLineRGBA(renderer, x1 + offsetx, y1 + offsety, x2 + offsetx, y2 + offsety, size, curcolor.r, curcolor.g,
                      curcolor.b, curcolor.a);
    }
    void drawRect(int x, int y, int w, int h) {
        x += offsetx;
        y += offsety;
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderDrawRect(renderer, &rect);
    }
    void fillRect(int x, int y, int w, int h) {
        x += offsetx;
        y += offsety;
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderFillRect(renderer, &rect);
    }
    void drawRect(rect r) {
        SDL_Rect rect = r;
        rect.x += offsetx;
        rect.y += offsety;
        SDL_RenderDrawRect(renderer, &rect);
    }
    void fillRect(rect r) {
        SDL_Rect rect = r;
        rect.x += offsetx;
        rect.y += offsety;
        SDL_RenderFillRect(renderer, &rect);
    }
    void fillRoundedRect(int x, int y, int w, int h, int r) {
        SDL_Rect rect = {x + offsetx + r, y + offsety, w - 2 * r, h};
        SDL_RenderFillRect(renderer, &rect);
        SDL_Rect rect2 = {x + offsetx, y + offsety + r, r, h - 2 * r};
        SDL_RenderFillRect(renderer, &rect2);
        SDL_Rect rect3 = {x + offsetx + w - r, y + offsety + r, r, h - 2 * r};
        SDL_RenderFillRect(renderer, &rect3);
        fillCircle(x + r, y + r, r);
        fillCircle(x + w - r - 1, y + r, r);
        fillCircle(x + r, y + h - r - 1, r);
        fillCircle(x + w - r - 1, y + h - r - 1, r);
    }
    void drawCircle(int centerX, int centerY, int radius) {
        circleRGBA(renderer, offsetx + centerX, offsety + centerY, radius, curcolor.r, curcolor.g, curcolor.b,
                   curcolor.a);
    }
    void fillCircle(int centerX, int centerY, int radius) {
        float x = offsetx + centerX;
        float y = offsety + centerY;
        const int segments = std::max(24, std::min(radius, 60));
        std::vector<SDL_Vertex> vertices(segments + 1);
        std::vector<int> indices(segments * 3);
        vertices[0] = {{x, y}, curcolor, {0.0f, 0.0f}};
        for (int i = 0; i < segments; ++i) {
            float angle = (i / static_cast<float>(segments)) * 2.0f * static_cast<float>(M_PI);
            vertices[i + 1] = {{x + radius * std::cos(angle), y + radius * std::sin(angle)}, curcolor, {0.0f, 0.0f}};
            indices[i * 3] = 0;
            indices[i * 3 + 1] = i + 1;
            indices[i * 3 + 2] = (i + 1) % segments + 1;
        }
        SDL_RenderGeometry(renderer, nullptr, vertices.data(), vertices.size(), indices.data(), indices.size());
    }
    void drawArc(int x, int y, int radius, float startAngle, float endAngle) {
        x += offsetx;
        y += offsety;
        arcRGBA(renderer, x, y, radius, startAngle, endAngle, curcolor.r, curcolor.g, curcolor.b, curcolor.a);
    }
    int loadImageOntoTexture(const std::string &path) {
        SDL_Texture *texture = IMG_LoadTexture(renderer, path.c_str());
        if (!texture) {
            std::cerr << "IMG_LoadTexture Error: " << IMG_GetError() << std::endl;
            return -1;
        }
        textures.push_back(texture);
        return textures.size() - 1;
    }
    void unloadTexture(int texture) {
        if (texture >= 0 && texture < textures.size() && textures[texture]) {
            SDL_DestroyTexture(textures[texture]);
            textures[texture] = nullptr;
        }
    }
    int loadFont(const std::string &fontPath, int fontSize) {
        TTF_Font *font = TTF_OpenFont(fontPath.c_str(), fontSize);
        if (!font) {
            std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
        }
        fonts.push_back(font);
        return fonts.size() - 1;
    }
    void drawText(const std::string &text, int x, int y, int ind, SDL_Color color) {
        if (ind < 0 || ind >= fonts.size() || !fonts[ind] || text.empty())
            return;
        x += offsetx;
        y += offsety;
        TTF_Font *font = fonts[ind];
        SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
        if (!surface)
            return;
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture)
            return;
        SDL_Rect destRect = {x, y, 0, 0};
        SDL_QueryTexture(texture, NULL, NULL, &destRect.w, &destRect.h);
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
        SDL_DestroyTexture(texture);
    }
    void present() {
        SDL_RenderPresent(renderer);
        frames++;
    }
    void clear(SDL_Color c) {
        setColor(c);
        SDL_RenderClear(renderer);
    }
    void clear() { SDL_RenderClear(renderer); }
};
