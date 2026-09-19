#include "drawing.hpp"
#include "utils.hpp"

int main() {
    Window w("sample",1000,500);
    while (w.isRunning()) {
        w.poll();
        w.clear(Color::White);
        w.setColor(Color::Red);
        w.fillRect(100,100,100,100);
        w.present();
        setFrameRate(60);
    }
}
