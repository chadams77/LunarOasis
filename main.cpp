#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#define MAX(_X, _Y) ((_X) > (_Y) ? (_X) : (_Y))
#define MIN(_X, _Y) ((_X) < (_Y) ? (_X) : (_Y))
#define CLAMP(_X, _A, _B) MIN(_B, MAX(_X, _A))
#define SPR(_SX, _SY, _W, _H) ((uint64_t)(_SX) | ((uint64_t)(_SY)<<10ul) | ((uint64_t)(_W)<<20ul) | ((uint64_t)(_H)<<30ul))
#define SPR_X(_C) ((int)((_C)&1023ul))
#define SPR_Y(_C) ((int)(((_C)>>10ul)&1023ul))
#define SPR_W(_C) ((int)(((_C)>>20ul)&1023ul))
#define SPR_H(_C) ((int)(((_C)>>30ul)&1023ul))

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
uint16_t * terrainBfr;

/* SPRITES */
const uint64_t ROCKS[] = {
    SPR(83, 5, 9, 9),
    SPR(102, 7, 4, 4),
    SPR(112, 1, 15, 13),
    SPR(83, 19, 9, 9),
    SPR(102, 22, 4, 4),
    SPR(113, 17, 15, 13),
    SPR(83, 36, 9, 9),
    SPR(102, 38, 4, 4),
    SPR(113, 34, 15, 13)
};
const int N_ROCKS = 9;

const uint64_t SHIP_OFF[] = {
    SPR(0, 0, 4, 9),
    SPR(42, 0, 6, 6),
    SPR(31, 10, 9, 4),
    SPR(41, 13, 6, 6),
    SPR(10, 0, 4, 9),
    SPR(58, 13, 6, 6),
    SPR(10, 10, 9, 4),
    SPR(59, 0, 6, 6)
};
const uint64_t SHIP_ON[] = {
    SPR(5, 0, 4, 9),
    SPR(49, 0, 9, 9),
    SPR(20, 20, 9, 4),
    SPR(48, 10, 9, 9),
    SPR(15, 0, 4, 9),
    SPR(68, 10, 9, 9),
    SPR(0, 10, 9, 4),
    SPR(6, 0, 9, 9)
};
const uint64_t PAL_SPR = SPR(0, 32, 9, 6);
uint32_t PAL_RED[9],
         PAL_GREEN[9],
         PAL_PINK[9],
         PAL_BLUE[9],
         PAL_BROWN[9],
         PAL_GREY[9];

/* SFX */
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
/* --- */

void clearBfr(uint32_t clr = 0xFF000000) {
    uint32_t * it = (uint32_t*)bfr64,
             * end = (uint32_t*)bfr64 + (64<<6);
    while (it != end) {
        *it = clr;
        it ++;
    }
}

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
        clearBfr(clr);
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

void drawSpr(uint64_t code, int x, int y) {
    drawSpr(SPR_X(code), SPR_Y(code), SPR_W(code), SPR_H(code), x, y);
}

void terrainClear() {
    memset((char *)terrainBfr, 0, sizeof(uint16_t) << 20);
}

void terrainAdd(uint64_t spr, int cx, int cy, int z, int scale = 100) { // scale = f * 100
    const int tx = SPR_X(spr),
              ty = SPR_Y(spr),
              tw = SPR_W(spr),
              th = SPR_H(spr);
    int x1 = cx - (tw / 2),
        y1 = cy - (th / 2);
    for (int x=x1; x<(x1+tw); x++) {
        if (x<0 || x>1023) {
            continue;
        }
        for (int y=y1; y<(y1+th); y++) {
            if (y<0 || y>1023) {
                continue;
            }
            uint32_t tclr = sprBfr[x - x1 + tx + ((y-y1+ty)<<10)];
            if (((tclr >> 24) & 0xFF) > 16u) {
                uint16_t c1 = CLAMP(z + ((int)(tclr & 0xFF) * scale / 100), (uint16_t)0, (uint16_t)(0xFFFF));
                uint16_t * ptr = terrainBfr + x + (y<<10);
                if (scale > 0) {
                    ptr[0] = MAX(ptr[0], c1);
                }
                else {
                    ptr[0] = MIN(ptr[0], c1);
                }
            }
        }
    }
}

void terrainRender(int cx, int cy) {
    uint32_t * it = (uint32_t*)bfr64;
    for (int sy=0; sy<64; sy++) {
        for (int sx=0; sx<64; sx++) {
            int x = cx - 32 + sx,
                y = cy - 32 + sy;
            if (x >= 0 && y >= 0 && x < 1024 && y < 1024) {
                int t00 = (int)terrainBfr[x + (y<<10)];
                int tp0 = x < 1023 ? (int)terrainBfr[x + 1 + (y<<10)] : t00;
                int tp0x = x < 1022 ? (int)terrainBfr[x + 2 + (y<<10)] : tp0;
                int tn0 = x > 0 ? (int)terrainBfr[x - 1 + (y<<10)] : t00;
                int tn0x = x > 1 ? (int)terrainBfr[x - 2 + (y<<10)] : tn0;
                int t0p = y < 1023 ? (int)terrainBfr[x + ((y+1)<<10)] : t00;
                int t0px = y < 1022 ? (int)terrainBfr[x + ((y+2)<<10)] : t0p;
                int t0n = y > 0 ? (int)terrainBfr[x + ((y-1)<<10)] : t00;
                int t0nx = y > 1 ? (int)terrainBfr[x + ((y-2)<<10)] : t0n;
                tp0 = (tp0 + tp0x) / 2;
                tn0 = (tn0 + tn0x) / 2;
                t0p = (t0p + t0px) / 2;
                t0n = (t0n + t0nx) / 2;
                if (t00 > 0) {
                    int l = t00;
                    if (tp0 > tn0) {
                        if (t00 > tn0) {
                            l = (l + l/2) / 2;
                        }
                        else {
                            l = (l + l/4) / 2;
                        }
                    }
                    else {
                        l = l + l / 5;
                    }
                    if (t0p < t0n) {
                        if (t00 > t0n) {
                            l = (l + l/2) / 2;
                        }
                        else {
                            l = (l + l/4) / 2;
                        }
                    }
                    else {
                        l = l + l / 5;
                    }
                    it[sx] = PAL_GREY[CLAMP((l*9)>>8, 1, 8)];
                }
            }
        }
        it += 64;
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

    for (int i=0; i<9; i++) {
        int x1 = SPR_X(PAL_SPR),
            y1 = SPR_Y(PAL_SPR);
        PAL_RED[i]   = sprBfr[x1 + i + ((y1+0) << 10)];
        PAL_GREEN[i] = sprBfr[x1 + i + ((y1+1) << 10)];
        PAL_PINK[i]  = sprBfr[x1 + i + ((y1+2) << 10)];
        PAL_BLUE[i]  = sprBfr[x1 + i + ((y1+3) << 10)];
        PAL_BROWN[i] = sprBfr[x1 + i + ((y1+4) << 10)];
        PAL_GREY[i]  = sprBfr[x1 + i + ((y1+5) << 10)];
    }

    terrainBfr = new uint16_t[1024*1024];
    terrainClear();

    for (int i=0; i<8192; i++) {
        terrainAdd(ROCKS[rand()%N_ROCKS], rand()&1023, rand()&1023, 128);
    }

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

        terrainRender(32, 32);

        drawSpr(SHIP_ON[0], 24, 24);

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

    delete terrainBfr;
    delete sprBfr;
    delete spritesTex;
    delete bfr64;
    delete tex64;
    delete spr64;
    delete window;

    return 0;
}