#pragma once
#include "components.hpp"
#include "drawing.hpp"
#include <vector>

class App;

enum class Filter {
    IGNORE, PASS, STOP
};

class Button : public Container {
  public:
    Container* child;
    SDL_Rect area;
    Button(double width, double height, Container* child = nullptr) 
        : Container(width, height), child(child), area({0, 0, 0, 0}) {}
    virtual void resize(double width, double height) override {
        absolute.width = width;
        absolute.height = height;
        if (child) {
            child->resize(width, height);
        }
    }
    virtual void render(Window &window, int x, int y) override {
        area = {x, y, static_cast<int>(absolute.width), static_cast<int>(absolute.height)};
        window.setColor(bg);
        window.fillRect(x, y, area.w, area.h);
        if (child) {
            child->render(window, x, y);
        }
    }
    virtual ~Button() {
        if (child) delete child;
    }
};

class ButtonHandler {
  public:
    Button* button;
    Filter filter;
    SDL_Rect area;
    void (*onClick)(App&);
    ButtonHandler(Button* btn, Filter flt, void (*handler)(App&))
        : button(btn), filter(flt), area({0, 0, 0, 0}), onClick(handler) {}
    Filter processClick(int mx, int my, App& app) {
        if (filter == Filter::IGNORE) return Filter::IGNORE;
        area = button->area;
        if (mx >= area.x && mx <= area.x + area.w &&
            my >= area.y && my <= area.y + area.h) {
            if (onClick) {
                onClick(app);
            }
            return filter;
        }
        return Filter::PASS;
    }
};

class App {
    bool was_clicked = false;
  public:
    std::vector<Container *> layers;
    std::vector<ButtonHandler> handlers;
    void update(Window& w) {
        bool is_clicked = w.isLeftClicked();
        if (is_clicked && !was_clicked) {
            vec2d mouse = w.mousePosition();
            for (auto it = handlers.rbegin(); it != handlers.rend(); ++it) {
                Filter result = it->processClick(mouse.x, mouse.y, *this);
                if (result == Filter::STOP) {
                    break;
                }
            }
        }
        was_clicked = is_clicked;
    }
    void resize(int width, int height) {
        for (auto i : layers) {
            i->resize(width, height);
        }
    }
    void render(Window& w) {
        for (auto i : layers) {
            i->render(w, 0, 0);
        }
    }
    ~App() {
        for (Container* c : layers) {
            delete c;
        }
    }
};
