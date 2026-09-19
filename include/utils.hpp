#pragma once
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>

namespace Color {
constexpr SDL_Color White = {255, 255, 255, 255};
constexpr SDL_Color Black = {0, 0, 0, 255};
constexpr SDL_Color Red = {255, 0, 0, 255};
constexpr SDL_Color Green = {0, 255, 0, 255};
constexpr SDL_Color Blue = {0, 0, 255, 255};

constexpr SDL_Color Yellow = {255, 255, 0, 255};
constexpr SDL_Color Cyan = {0, 255, 255, 255};
constexpr SDL_Color Magenta = {255, 0, 255, 255};

constexpr SDL_Color LightGray = {192, 192, 192, 255};
constexpr SDL_Color Gray = {128, 128, 128, 255};
constexpr SDL_Color DarkGray = {64, 64, 64, 255};

constexpr SDL_Color Orange = {255, 165, 0, 255};
constexpr SDL_Color Purple = {128, 0, 128, 255};
constexpr SDL_Color Pink = {255, 192, 203, 255};
constexpr SDL_Color Brown = {165, 42, 42, 255};

constexpr SDL_Color Transparent = {0, 0, 0, 0};
} // namespace Color

inline bool operator==(const SDL_Rect &one, const SDL_Rect &other) {
    return one.x == other.x && one.y == other.y && one.w == other.w && one.h == other.h;
}
inline bool operator!=(const SDL_Rect &one, const SDL_Rect &other) {
    return one.x != other.x || one.y != other.y || one.w != other.w || one.h != other.h;
}
inline int integralSquareRoot(int num) {
    if (num < 0)
        return -1;
    if (num == 0 || num == 1)
        return num;
    int low = 1, high = num, result = 0;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (mid == num / mid) {
            return mid;
        } else if (mid < num / mid) {
            result = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return result;
}
inline double calcAngle(int x1, int y1, int x2, int y2) {
    return fmod(atan2(y2 - y1, x2 - x1) * (180.0 / M_PI) + 360.0, 360.0);
}
inline void setFrameRate(int targetFPS) {
    static Uint32 lastTime = 0;
    Uint32 frameTime = 1000 / targetFPS;
    Uint32 currentTime = SDL_GetTicks();
    Uint32 deltaTime = currentTime - lastTime;
    if (deltaTime < frameTime) {
        SDL_Delay(frameTime - deltaTime);
    }
    lastTime = SDL_GetTicks();
}
struct vec2d {
    float x, y;
    vec2d(float x = 0.f, float y = 0.f) : x(x), y(y) {}
    vec2d operator+(vec2d v) const { return {x + v.x, y + v.y}; }
    vec2d operator+=(vec2d v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    vec2d operator-(vec2d v) const { return {x - v.x, y - v.y}; }
    vec2d operator-=(vec2d v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    vec2d operator*(float v) const { return {x * v, y * v}; }
    vec2d operator*=(float v) {
        x *= v;
        y *= v;
        return *this;
    }
    vec2d operator/(float v) const { return {x / v, y / v}; }
    vec2d operator/=(float v) {
        x /= v;
        y /= v;
        return *this;
    }
    float magnitude() const { return std::sqrt(x * x + y * y); }
    vec2d dirn() const { return *this / magnitude(); }
    bool normalise() {
        float mag = magnitude();
        if (mag == 0)
            return false;
        x /= mag;
        y /= mag;
        return true;
    }
    float dot(vec2d v) const { return x * v.x + y * v.y; }
    float cross(vec2d v) const { return x * v.y - y * v.x; }
};
struct circle {
    vec2d center;
    float radius;
    circle(vec2d center = vec2d(), float radius = 0.f) : center(center), radius(radius) {}
};
struct rect {
    float x, y, w, h;
    rect(float x = 0.0f, float y = 0.0f, float w = 0.0f, float h = 0.0f) : x(x), y(y), w(w), h(h) {}
    rect(const SDL_Rect &sr)
        : x(static_cast<float>(sr.x)), y(static_cast<float>(sr.y)), w(static_cast<float>(sr.w)),
          h(static_cast<float>(sr.h)) {}
    operator SDL_Rect() const {
        return {static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h)};
    }
    bool checkCollision(const rect &r) const {
        if (x + w > r.x && r.x + r.w > x && y + h > r.y && r.y + r.h > y)
            return true;
        return false;
    }
    bool checkCollision(const circle &c) const {
        vec2d nearestPoint;
        nearestPoint.x = std::max(x, std::min(c.center.x, x + w));
        nearestPoint.y = std::max(y, std::min(c.center.y, y + h));
        vec2d rayToNearest = nearestPoint - c.center;
        float magSq = (rayToNearest.x * rayToNearest.x) + (rayToNearest.y * rayToNearest.y);
        return magSq <= (c.radius * c.radius);
    }
    bool resolve(circle &c) const {
        vec2d nearestPoint;
        nearestPoint.x = std::max(x, std::min(c.center.x, x + w));
        nearestPoint.y = std::max(y, std::min(c.center.y, y + h));
        vec2d rayToNearest = nearestPoint - c.center;
        float magSq = (rayToNearest.x * rayToNearest.x) + (rayToNearest.y * rayToNearest.y);
        if (magSq > c.radius * c.radius)
            return false;
        float mag = std::sqrt(magSq);
        if (mag > 0.0f) {
            float overlap = c.radius - mag;
            c.center.x -= (rayToNearest.x / mag) * overlap;
            c.center.y -= (rayToNearest.y / mag) * overlap;
        } else {
            float distLeft = c.center.x - x;
            float distRight = (x + w) - c.center.x;
            float distTop = c.center.y - y;
            float distBottom = (y + h) - c.center.y;
            float minDist = std::min({distLeft, distRight, distTop, distBottom});
            if (minDist == distLeft)
                c.center.x = x - c.radius;
            else if (minDist == distRight)
                c.center.x = x + w + c.radius;
            else if (minDist == distTop)
                c.center.y = y - c.radius;
            else
                c.center.y = y + h + c.radius;
        }
        return true;
    }
    bool resolve(rect &r) const {
        if (!checkCollision(r))
            return false;
        float thisCX = x + w / 2.0f;
        float thisCY = y + h / 2.0f;
        float rCX = r.x + r.w / 2.0f;
        float rCY = r.y + r.h / 2.0f;
        float dx = rCX - thisCX;
        float dy = rCY - thisCY;
        float overlapX = (w / 2.0f + r.w / 2.0f) - std::abs(dx);
        float overlapY = (h / 2.0f + r.h / 2.0f) - std::abs(dy);
        if (overlapX < overlapY) {
            if (dx > 0)
                r.x += overlapX;
            else
                r.x -= overlapX;
        } else {
            if (dy > 0)
                r.y += overlapY;
            else
                r.y -= overlapY;
        }
        return true;
    }
};
