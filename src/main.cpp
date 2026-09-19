#include "components.hpp"
#include "control.hpp"
#include "drawing.hpp"
#include "utils.hpp"

SDL_Color currentColor = Color::Black;
bool isEraser = false;

void setRed(App &app) {
    currentColor = Color::Red;
    isEraser = false;
}
void setGreen(App &app) {
    currentColor = Color::Green;
    isEraser = false;
}
void setBlue(App &app) {
    currentColor = Color::Blue;
    isEraser = false;
}
void setBlack(App &app) {
    currentColor = Color::Black;
    isEraser = false;
}
void setYellow(App &app) {
    currentColor = Color::Yellow;
    isEraser = false;
}
void setPen(App &app) { isEraser = false; }
void setEraser(App &app) { isEraser = true; }

int main() {
    Window w("Drawing App", 800, 600);

    int canvasTex = w.createTargetTexture(700, 600);
    w.setRenderTarget(canvasTex);
    w.clear(Color::White);
    w.resetRenderTarget();
    int font = w.loadFont("/usr/share/fonts/FiraCode-Medium.ttf",10);

    App app;

    HBox *root = new HBox(1.0, 1.0);
    VBox *toolbar = new VBox(100, 1.0);

    Button *btnPen = new Button(1.0, -1, new Label(-1, -1, "Pen", font));
    btnPen->bg = Color::DarkGray;
    Button *btnEraser = new Button(1.0, -1, new Label(-1, -1, "Eraser", font));
    btnEraser->bg = Color::LightGray;
    Button *btnC1 = new Button(1.0, -1);
    btnC1->bg = Color::Red;
    Button *btnC2 = new Button(1.0, -1);
    btnC2->bg = Color::Green;
    Button *btnC3 = new Button(1.0, -1);
    btnC3->bg = Color::Blue;
    Button *btnC4 = new Button(1.0, -1);
    btnC4->bg = Color::Black;
    Button *btnC5 = new Button(1.0, -1);
    btnC5->bg = Color::Yellow;

    toolbar->add(btnPen);
    toolbar->add(btnEraser);
    toolbar->add(btnC1);
    toolbar->add(btnC2);
    toolbar->add(btnC3);
    toolbar->add(btnC4);
    toolbar->add(btnC5);

    app.handlers.push_back(ButtonHandler(btnPen, Filter::STOP, setPen));
    app.handlers.push_back(ButtonHandler(btnEraser, Filter::STOP, setEraser));
    app.handlers.push_back(ButtonHandler(btnC1, Filter::STOP, setRed));
    app.handlers.push_back(ButtonHandler(btnC2, Filter::STOP, setGreen));
    app.handlers.push_back(ButtonHandler(btnC3, Filter::STOP, setBlue));
    app.handlers.push_back(ButtonHandler(btnC4, Filter::STOP, setBlack));
    app.handlers.push_back(ButtonHandler(btnC5, Filter::STOP, setYellow));

    TextureContainer *canvas = new TextureContainer(-1, 1.0, canvasTex);

    root->add(toolbar);
    root->add(canvas);

    app.layers.push_back(root);
    app.resize(800, 600);

    bool wasDrawing = false;
    vec2d lastMouse(0, 0);

    while (w.isRunning()) {
        w.poll();
        app.update(w);

        vec2d mouse = w.mousePosition();
        bool isCanvasHovered = (mouse.x >= 100 && mouse.x <= 800 && mouse.y >= 0 && mouse.y <= 600);

        if (w.isLeftClicked()) {
            if (isCanvasHovered) {
                float targetX = mouse.x - 100;
                float targetY = mouse.y;

                w.setRenderTarget(canvasTex);
                w.setColor(isEraser ? Color::White : currentColor);
                w.fillCircle(targetX, targetY, 4);

                if (wasDrawing) {
                    w.drawThickLine(lastMouse.x, lastMouse.y, targetX, targetY, 8);
                }
                w.resetRenderTarget();

                lastMouse = vec2d(targetX, targetY);
                wasDrawing = true;
            } else {
                wasDrawing = false;
            }
        } else {
            wasDrawing = false;
        }

        w.clear(Color::White);
        app.render(w);
        w.present();
        setFrameRate(60);
    }

    return 0;
}
