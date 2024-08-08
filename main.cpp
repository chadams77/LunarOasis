#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace sf;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::set;

RenderWindow * window = NULL;
Texture * tex64 = NULL;
Sprite * spr64 = NULL;
uint8_t * bfr64 = NULL;
Texture * spritesTex = NULL;
const uint32_t * sprBfr;

const int MAX_SOUNDS = 64;
int soundIdx = 0;
Sound sounds[MAX_SOUNDS];

void playSound(SoundBuffer & bfr, double rate=1., double vol=1.) {
    sounds[soundIdx].setBuffer(bfr);
    sounds[soundIdx].setVolume(vol*100.);
    sounds[soundIdx].setPitch(rate);
    sounds[soundIdx].play();
    soundIdx = (soundIdx + 1) % MAX_SOUNDS;
}

void clearBfr(uint32_t clr = 0xFF000000) {
    uint32_t * it = (uint32_t*)bfr64,
             * end = (uint32_t*)bfr64 + (64<<6);
    while (it != end) {
        *it = clr;
        it ++;
    }
}

#define MAX(_X, _Y) ((_X) > (_Y) ? (_X) : (_Y))
#define MIN(_X, _Y) ((_X) < (_Y) ? (_X) : (_Y))
#define CLAMP(_X, _A, _B) MIN(_B, MAX(_X, _A))

void drawBox(int _x1, int _y1, int _w, int _h, uint32_t clr) {
    if (_x1 >= 64 || _y1 >= 64 || _w <= 0 || _h <= 0 || (_x1 + _w) <= 0 || (_y1 + _h) <= 0) {
        return;
    }
    int x1 = CLAMP(_x1, 0, 63),
        y1 = CLAMP(_y1, 0, 63),
        x2 = CLAMP(_x1 + _w, 0, 64),
        y2 = CLAMP(_y1 + _h, 0, 64);
    uint32_t * it = (uint32_t*)bfr64 + (y1 << 6);
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            it[x] = clr;
        }
        it += 64;
    }
}

void drawCircle(int x, int y, int r, uint32_t clr) {
    if ((x+r) < 0 || (y+r) < 0 || (x-r) > 63 || (y-r) > 63) {
        return;
    }
    int x1 = CLAMP(x-r,0,63),
        y1 = CLAMP(y-r,0,63),
        x2 = CLAMP(x+r,0,63),
        y2 = CLAMP(y+r,0,63);
    const int r2 = r*r;
    for (int xx=x1; xx<=x2; xx++) {
        for (int yy=y1; yy<=y2; yy++) {
            int d2 = (xx-x)*(xx-x)+(yy-y)*(yy-y);
            if (d2 <= r2) {
                ((uint32_t*)bfr64)[xx+(yy<<6)] = clr;
            }
        }
    }
}

void drawNotCircle(int x, int y, int r, uint32_t clr) {
    const int r2 = r*r;
    for (int xx=0; xx<64; xx++) {
        for (int yy=0; yy<64; yy++) {
            int d2 = (xx-x)*(xx-x)+(yy-y)*(yy-y);
            if (d2 > r2) {
                ((uint32_t*)bfr64)[xx+(yy<<6)] = clr;
            }
        }
    }
}

void drawSpr(int _sx, int _sy, int _w, int _h, int dx, int dy) {
    if (dx >= 64 || dy >= 64 || _w <= 0 || _h <= 0 || (dx + _w) <= 0 || (dy + _h) <= 0) {
        return;
    }
    uint32_t * it = (uint32_t*)bfr64 + (dy << 6);
    uint32_t * its = (uint32_t*)sprBfr + (_sy << 10);
    for (int y=0; y<_h; y++) {
        if ((y+dy) < 0 || (y+dy) > 63) {
            it += 64; its += 1024;
            continue;
        }
        for (int x=0; x<_w; x++) {
            if ((x+dx) < 0 || (x+dx) > 63) {
                continue;
            }
            it[dx+x] = its[x+_sx];
        }
        it += 64; its += 1024;
    }
}

int main() {

    bool fullscreen = false;

    window = new RenderWindow(VideoMode(800, 600), "Lunar Oasis");
    window->setMouseCursorVisible(false);

    window->setFramerateLimit(60);

    tex64 = new Texture();
    tex64->create(64, 64);
    tex64->setSmooth(false);

    bfr64 = new uint8_t[64*64*4];

    clearBfr(0xFF0000FF);

    tex64->update(bfr64);

    spr64 = new Sprite(*tex64);

    spritesTex = new Texture();
    if (!spritesTex->loadFromFile("sprites/sprite-sheet.png")) {
        cerr << "Tileset not found" << endl;
        exit(0);
    }
    Image spritesImg = spritesTex->copyToImage();
    sprBfr = (const uint32_t*)spritesImg.getPixelsPtr();

    double time = 0.;
    
    while (window->isOpen()) {
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed) {
                window->close();
            }
            else if (event.type == Event::Resized) {
	            window->setView(View(FloatRect(0.f, 0.f, (float)window->getSize().x, (float)window->getSize().y)));
            }
            else if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Key::F11) {
                    fullscreen = !fullscreen;
                    delete window;
                    window = new RenderWindow(fullscreen ? VideoMode::getDesktopMode() : VideoMode(800, 600), "Lunar Oasis", fullscreen ? Style::Fullscreen : Style::Default);
                    window->setFramerateLimit(60);
	                window->setView(View(FloatRect(0.f, 0.f, (float)window->getSize().x, (float)window->getSize().y)));
                }
            }
        }

        clearBfr();

        //


        //

        tex64->update(bfr64);

        window->clear(Color::Black);

        spr64->setOrigin(Vector2f(32.f, 32.f));
        spr64->setPosition(Vector2f((float)window->getSize().x, (float)window->getSize().y) * 0.5f);
        float scale = 1.f;
        if (window->getSize().x > window->getSize().y) {
            scale = window->getSize().y / 64.f;
        }
        else {
            scale = window->getSize().x / 64.f;
        }
        spr64->setScale(Vector2f(scale, scale));

        window->draw(*spr64);

        double dt = 1. / 60.;

        time += dt;

        window->display();
    }

    delete sprBfr;
    delete spritesTex;
    delete bfr64;
    delete tex64;
    delete spr64;
    delete window;

    return 0;
}