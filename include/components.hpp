#pragma once
#include "drawing.hpp"
#include "utils.hpp"
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

inline bool adjust(std::vector<double> &relative, std::vector<std::reference_wrapper<double>> &absolute,
                   double length) {
    size_t total_expand = 0, total_size = 0;
    for (double f : relative) {
        if (f == -1)
            total_expand++;
        else if (f < 0)
            return false;
        else if (f >= 0 && f <= 1) {
            total_size += length * f;
        } else {
            total_size += f;
        }
    }
    if (total_size > length)
        return false;
    double expanded = length - total_size;
    if (total_expand > 0)
        expanded /= total_expand;
    for (size_t i = 0; i < relative.size(); i++) {
        if (relative[i] == -1)
            absolute[i].get() = expanded;
        else if (relative[i] >= 0 && relative[i] <= 1)
            absolute[i].get() = (length * relative[i]);
        else
            absolute[i].get() = relative[i];
    }
    return true;
}

class dimension {
  public:
    double width, height;
    dimension(double width = 0, double height = 0) : width(width), height(height) {}
};

class Container {
  public:
    dimension absolute, relative;
    SDL_Color bg, fg;

    Container(double width, double height) : relative(width, height) {}
    virtual ~Container() = default;
    virtual void resize(double width, double height) = 0;
    virtual void render(Window &window, int x = 0, int y = 0) = 0;
};

class Label : public Container {
  public:
    std::string text;
    int font;

    Label(double width, double height, std::string text, int font) : Container(width, height), text(text), font(font) {}
    virtual void resize(double width, double height) override {
        absolute.width = width;
        absolute.height = height;
    }
    virtual void render(Window &window, int x, int y) override {
        window.drawText(text, static_cast<int>(x), static_cast<int>(y), font, fg);
    }
};

class TextureContainer : public Container {
  public:
    int texture;

    TextureContainer(double width, double height, int texture) : Container(width, height), texture(texture) {}
    virtual void resize(double width, double height) override {
        absolute.width = width;
        absolute.height = height;
    }
    virtual void render(Window &window, int x, int y) override {
        SDL_Rect win = {static_cast<int>(x), static_cast<int>(y), static_cast<int>(absolute.width),
                        static_cast<int>(absolute.height)};
        window.drawTexture(texture, win);
    }
};

class Empty : public Container {
  public:
    Empty(double width = 0, double height = 0) : Container(width, height) {}
    virtual void resize(double width, double height) override {
        absolute.width = width;
        absolute.height = height;
    }
    virtual void render(Window &window, int x, int y) override {}
};

class HBox : public Container {
    std::vector<double> child_relative;
    std::vector<std::reference_wrapper<double>> child_absolute;
    std::vector<Container *> containers;

  public:
    HBox(double width, double height, std::initializer_list<Container *> init_containers = {})
        : Container(width, height) {
        for (Container *c : init_containers) {
            add(c);
        }
    }
    virtual ~HBox() {
        for (Container *c : containers) {
            delete c;
        }
    }
    void add(Container *c) {
        containers.push_back(c);
        child_relative.push_back(c->relative.width);
        child_absolute.push_back(std::ref(c->absolute.width));
    }

    virtual void resize(double width, double height) override {
        absolute.width = width;
        absolute.height = height;

        for (size_t i = 0; i < containers.size(); i++) {
            child_relative[i] = containers[i]->relative.width;
        }

        if (adjust(child_relative, child_absolute, width)) {
            for (Container *c : containers) {
                c->resize(c->absolute.width, height);
            }
        }
    }

    virtual void render(Window &window, int x, int y) override {
        double current_x = x;
        for (Container *c : containers) {
            c->render(window, current_x, y);
            current_x += c->absolute.width;
        }
    }
};

class VBox : public Container {
    std::vector<double> child_relative;
    std::vector<std::reference_wrapper<double>> child_absolute;

  public:
    std::vector<Container *> containers;

    VBox(double width, double height, std::initializer_list<Container *> init_containers = {})
        : Container(width, height) {
        for (Container *c : init_containers) {
            add(c);
        }
    }
    virtual ~VBox() {
        for (Container* c : containers) {
            delete c;
        }
    }
    void add(Container *c) {
        containers.push_back(c);
        child_relative.push_back(c->relative.height);
        child_absolute.push_back(std::ref(c->absolute.height));
    }

    virtual void resize(double width, double height) override {
        absolute.width = width;
        absolute.height = height;

        for (size_t i = 0; i < containers.size(); i++) {
            child_relative[i] = containers[i]->relative.height;
        }

        if (adjust(child_relative, child_absolute, height)) {
            for (Container *c : containers) {
                c->resize(width, c->absolute.height);
            }
        }
    }

    virtual void render(Window &window, int x, int y) override {
        double current_y = y;
        for (Container *c : containers) {
            c->render(window, x, current_y);
            current_y += c->absolute.height;
        }
    }
};
