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
#define PI 3.14159265359f

#define PLAYER_THRUST     15.f
#define PLAYER_TURN_SPEED 4.5f
#define GRAVITY           6.5f

using namespace sf;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::set;

struct prtType {
    int id;
    float x, y, xv, yv;
    uint32_t * pal;
    float energy;
    float mass;
    float life;
    float shadef;
    prtType * next;
};
prtType ** phash;
prtType * plist;
const int MAX_PRT = 2048;

RenderWindow * window = NULL;
Texture * tex64 = NULL;
Sprite * spr64 = NULL;
uint8_t * bfr64 = NULL;
Texture * spritesTex = NULL;
const uint32_t * sprBfr;
uint16_t * terrainBfr = NULL;
uint8_t * tspecBfr = NULL;

float playerX, playerY, playerVX, playerVY, playerAngle, playerFuel;
bool playerDead;
float flagX, flagY, flagH, flagVis;

struct depotType {
    bool exists;
    float x, y, fuel;
};

const int MAX_DEPOT = 16;
depotType depots[MAX_DEPOT];

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

const uint64_t EX_HUGE = SPR(128+16, 0, 32, 32);
const uint64_t EX_BIG = SPR(128, 0, 16, 16);
const uint64_t EX_SMALL = SPR(128, 16, 16, 16);

const uint64_t DEPOT_FRAMES[] = {
    SPR(0, 96, 5, 7),
    SPR(5, 96, 5, 7),
    SPR(10, 96, 5, 7),
    SPR(15, 96, 5, 7),
    SPR(20, 96, 5, 7)
};

const uint64_t FLAG_FRAMES[] = {
    SPR(0, 104, 5, 8),
    SPR(5, 104, 5, 8),
    SPR(10, 104, 5, 8),
    SPR(15, 104, 5, 8)
};

const uint64_t SHIP_OFF[] = {
    SPR(0*16, 3*16, 16, 16),
    SPR(2*16, 3*16, 16, 16),
    SPR(4*16, 3*16, 16, 16),
    SPR(6*16, 3*16, 16, 16),
    SPR(0*16, 4*16, 16, 16),
    SPR(2*16, 4*16, 16, 16),
    SPR(4*16, 4*16, 16, 16),
    SPR(6*16, 4*16, 16, 16)
};
const uint64_t SHIP_ON[] = {
    SPR(1*16, 3*16, 16, 16),
    SPR(3*16, 3*16, 16, 16),
    SPR(5*16, 3*16, 16, 16),
    SPR(7*16, 3*16, 16, 16),
    SPR(1*16, 4*16, 16, 16),
    SPR(3*16, 4*16, 16, 16),
    SPR(5*16, 4*16, 16, 16),
    SPR(7*16, 4*16, 16, 16)
};
const uint64_t SHIP_LANDED = SPR(0*16, 5*16, 16, 16);
const uint64_t BG_SPR[] = {
    SPR(176, 0, 64, 64),
    SPR(256, 0, 64, 64)
};
const uint64_t PAL_SPR = SPR(0, 32, 9, 6);
uint32_t PAL_RED[9],
         PAL_GREEN[9],
         PAL_PINK[9],
         PAL_BLUE[9],
         PAL_BROWN[9],
         PAL_GREY[9];

const uint64_t LEVEL_BG_1 = BG_SPR[0];
const int LEVEL_START_X_1 = 4, LEVEL_START_Y_1 = 2;
const uint8_t LEVEL_GRID_1[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,3,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,3,0,0,1,1,1,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,0,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8_t* LEVELS[] = {
    LEVEL_GRID_1
};
const uint64_t LEVEL_BG[] = {
    LEVEL_BG_1
};
const int LEVEL_START_X[] {
    LEVEL_START_X_1
};
const int LEVEL_START_Y[] {
    LEVEL_START_Y_1
};
const int N_LEVELS = 1;

int curLevel = 1;

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

static uint32_t blend(uint32_t bg, uint32_t clr) {
    int a = (bg >> 24) & 0xFF,
        r = (bg) & 0XFF,
        g = (bg >> 8) & 0xFF,
        b = (bg >> 16) & 0xFF;
    int ca = (clr >> 24) & 0xFF,
        cr = (clr) & 0XFF,
        cg = (clr >> 8) & 0xFF,
        cb = (clr >> 16) & 0xFF;
    a = CLAMP(a + ca, 0, 255);
    r = CLAMP(((r * (255 - ca)) >> 8) + ((cr * ca) >> 8), 0, 255);
    g = CLAMP(((g * (255 - ca)) >> 8) + ((cg * ca) >> 8), 0, 255);
    b = CLAMP(((b * (255 - ca)) >> 8) + ((cb * ca) >> 8), 0, 255);
    return (uint32_t)((((uint32_t)a) << 24u) | ((uint32_t)r) | (((uint32_t)g) << 8u) | (((uint32_t)b) << 16u));
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
            uint64_t clr = its[x+_sx];
            if (((clr>>24)&0xFF) > 0) {
                it[dx+x] = blend(it[dx+x], clr);
            }
        }
        it += 64; its += 1024;
    }
}

void drawSpr(uint64_t code, int x, int y) {
    drawSpr(SPR_X(code), SPR_Y(code), SPR_W(code), SPR_H(code), x, y);
}

bool sprCollideTerrain(int _sx, int _sy, int _w, int _h, int dx, int dy) {
    uint16_t * it = (uint16_t*)terrainBfr + (dy << 10);
    uint32_t * its = (uint32_t*)sprBfr + (_sy << 10);
    for (int y=0; y<_h; y++) {
        if ((y+dy) < 0 || (y+dy) > 511) {
            it += 1024; its += 1024;
            continue;
        }
        for (int x=0; x<_w; x++) {
            if ((x+dx) < 0 || (x+dx) > 511) {
                continue;
            }
            uint64_t clr = its[x+_sx];
            if (((clr>>24)&0xFF) > 0) {
                if (it[dx+x] > 0) {
                    return true;
                }
            }
        }
        it += 1024; its += 1024;
    }
    return false;
}

bool sprCollideTerrain(uint64_t code, int x, int y) {
    return sprCollideTerrain(SPR_X(code), SPR_Y(code), SPR_W(code), SPR_H(code), x, y);
}

void terrainClear() {
    memset((char *)terrainBfr, 0, sizeof(uint16_t) << 20);
    memset((char *)tspecBfr, 0, sizeof(uint8_t) << 20);
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
                uint16_t c1 = (uint16_t)(CLAMP(z + ((int)(tclr & 0xFF) * scale / 100), 0, 0xFFFF));
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
                tp0 = (tp0 * 2 + tp0x) / 3;
                tn0 = (tn0 * 2 + tn0x) / 3;
                t0p = (t0p * 2 + t0px) / 3;
                t0n = (t0n * 2 + t0nx) / 3;
                int shade = 6;
                if (t00 > 0) {
                    int l = t00;
                    int xa = 2 * (tp0 - tn0),
                        ya = 2 * (t0p - t0n),
                        za = -4;
                    int len = xa*xa+ya*ya+za*za;
                    int dot = ((ya - za - xa) * 65535) / len;
                    int shade = CLAMP(dot / 64 + 4, 2, 7);
                    it[sx] = PAL_GREY[shade];
                }
                else if (tspecBfr[x+(y<<10)] == 1) {
                    it[sx] = blend(it[sx], (PAL_GREY[2] & 0x00FFFFFF) | 0x50000000);
                }
            }
        }
        it += 64;
    }
}

void clearParticles() {
    memset(plist, 0, sizeof(prtType) * MAX_PRT);
    memset(phash, 0, sizeof(prtType*) * 512 * 512);
}

void addParticle(prtType p) {
    for (int i=0; i<MAX_PRT; i++) {
        if (plist[i].life <= 0.f) {
            plist[i] = p;
            plist[i].next = NULL;
            plist[i].id = i;
            return;
        }
    }
}

void addFire(float x, float y, float xv, float yv, int cnt = 4, float lifef = 1.0f) {
    prtType p;
    p.pal = PAL_RED;
    p.shadef = 1. / lifef;
    p.life = lifef * (float)((rand() & 0xF) + 32) / 16.f;
    p.mass = 0.1f;
    p.energy = 10.f;
    p.x = x + (float)(rand() & 0xFF) / 255.f - 0.5f;
    p.y = y + (float)(rand() & 0xFF) / 255.f - 0.5f;
    p.xv = xv;
    p.yv = yv;
    for (int i=0; i<cnt; i++) {
        addParticle(p);
    }
}

void explosion(float x, float y, float xv, float yv, int cnt) {
    float fs = (float)cnt / 256.f;
    for (int k=0; k<cnt; k++) {
        float vx = 3.f * ((float)(rand() & 0xFF) / 255.f - 0.5f);
        float vy = 3.f * ((float)(rand() & 0xFF) / 255.f - 0.5f);
        addFire(x + vx, y + vy, xv + vx * 15.f * fs, yv + vy * 50.f * fs, 4, 2.5f);
    }
}

void updateRenderParticles(float dt, int cx, int cy) {
    memset(phash, 0, sizeof(prtType*) * 512 * 512);
    for (int i=0; i<MAX_PRT; i++) {
        plist[i].next = NULL;
        if (plist[i].life > 0.f) {
            int hx = (int)floor(plist[i].x), hy = (int)floor(plist[i].y);
            if (hx >= 0 && hy >= 0 && hx < 512 && hy < 512) {
                int hi = hx + (hy << 9);
                plist[i].next = phash[hi];
                phash[hi] = plist + i;
            }
        }
    }
    for (int i=0; i<MAX_PRT; i++) {
        if (plist[i].life > 0.f) {
            plist[i].life -= dt;
            if (plist[i].life < 0.f) {
                plist[i].life = 0.f;
            }
            else {
                plist[i].xv -= plist[i].xv * dt * 0.25f;
                plist[i].yv -= plist[i].yv * dt * 0.25f;
                plist[i].yv += dt * GRAVITY;
                int hx = (int)floor(plist[i].x), hy = (int)floor(plist[i].y);
                for (int x=hx-1; x<=hx+1; x++) {
                    for (int y=hy-1; y<=hy+1; y++) {
                        if (x>=0 && y>=0 && x<512 && y<512) {
                            prtType * n = phash[x+(y<<9)];
                            while (n != NULL) {
                                if (n->id != plist[i].id) {
                                    double dx = plist[i].x - n->x,
                                           dy = plist[i].y - n->y;
                                    double m1 = plist[i].mass, m2 = n->mass;
                                    double len = sqrt(dx*dx+dy*dy) + 0.1;
                                    dx /= len;
                                    dy /= len;
                                    if (len < 1.) {
                                        double force = pow(1. / len, 3.);
                                        plist[i].xv += dx * force * (m2 / (m1 + m2)) * dt;
                                        plist[i].yv += dy * force * (m2 / (m1 + m2)) * dt;
                                        n->xv -= dx * force * (m1 / (m1 + m2)) * dt;
                                        n->yv -= dy * force * (m1 / (m1 + m2)) * dt;
                                    }
                                }
                                n = n->next;
                            }
                        }
                    }
                }
            }
        }
    }
    uint32_t * bfr = (uint32_t*)bfr64;
    for (int i=0; i<MAX_PRT; i++) {
        if (plist[i].life > 0.f) {
            float ox = plist[i].x, oy = plist[i].y;
            plist[i].x += plist[i].xv * dt;
            plist[i].y += plist[i].yv * dt;
            int x = (int)floor(plist[i].x) - cx + 32,
                y = (int)floor(plist[i].y) - cy + 32;
            if (x >= 0 && y >= 0 && x < 64 && y < 64) {
                int off = x + (y << 6);
                bfr[off] = blend(bfr[off], (plist[i].pal[CLAMP((int)floor(plist[i].life * plist[i].shadef * 3.), 1, 7)] & 0x00FFFFFF) | (CLAMP((uint32_t)floor(plist[i].life * 255.), 0, 255) << 24u));
            }
            int hx = (int)floor(plist[i].x), hy = (int)floor(plist[i].y);
            if (hx < 0 || hy < 0 || hx >= 512 || hy >= 512) {
                plist[i].life = 0.f;
            }
            else if (terrainBfr[hx + (hy << 10)] > 0) {
                plist[i].x = ox;
                plist[i].y = oy;
                if (fabs(plist[i].yv) > fabs(plist[i].xv)) {
                    plist[i].yv = -plist[i].yv * 0.5f;
                }
                else {
                    plist[i].xv = -plist[i].xv * 0.5f;
                }
            }
        }
    }
}

void initLevel(int _levelNo) {
    const int idx = _levelNo - 1;
    
    curLevel = _levelNo;
    terrainClear();
    clearParticles();

    const uint8_t * grid = LEVELS[idx];

    srand(_levelNo * 100);

    int tnz = 0;
    int depotI = 0;
    for (int x=0; x<64; x++) {
        for (int y=0; y<64; y++) {
            const int v = grid[x+(y<<6)];
            if (v == 1) { // rock
                tnz += 1;
            }
            else if (v == 2) { // flag
                flagX = 4.f + 8.f * (float)x;
                flagY = 4.f + 8.f * (float)y;
                flagH = 0.f;
                flagVis = true;
            }
            else if (v == 3) { // fuel depot
                if (depotI < MAX_DEPOT) {
                    depots[depotI].exists = true;
                    depots[depotI].fuel = 1.;
                    depots[depotI].x = 4.f + 8.f * (float)x;
                    depots[depotI].y = 4.f + 8.f * (float)y;
                    depotI += 1;
                }
            }
        }
    }

    for (int i=0; i<(tnz<<5); i++) {
        int cx = rand()&511,
            cy = rand()&511;
        if (grid[(cx>>3)+((cy>>3)<<6)] != 1) {
            i --;
            continue;
        }
        uint64_t spr = ROCKS[rand()%N_ROCKS];
        int w = SPR_W(spr),
            h = SPR_H(spr);
        bool any = false;
        for (int x=cx-(w>>1); x<cx+(w>>1); x++) {
            for (int y=cy-(h>>1); y<cy+(h>>1); y++) {
                if (x>=0 && y>=0) {
                    int lx = x>>3,
                        ly = y>>3;
                    if (lx < 64 && ly < 64) {
                        if (grid[lx+(ly<<6)] != 1) {
                            any = true;
                        }
                    }
                    else {
                        any = true;
                    }
                }
                else {
                    any = true;
                }
            }
        }
        if (!any) {
            terrainAdd(spr, cx, cy, 64 + (rand() & 63));
        }
    }

    for (int i=0; i<((1024<<10)>>7); i++) {
        long j = (long)((rand() << 15l) + rand()) & ((1l << 20l)-1l);
        tspecBfr[j] = 1;
    }

    playerX = (float)(LEVEL_START_X[idx] * 8 + 4);
    playerY = (float)(LEVEL_START_Y[idx] * 8 + 4);
    playerVX = 0.f;
    playerVY = 0.f;
    playerAngle = 0.f;
    playerDead = false;
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
    plist = new prtType[MAX_PRT];
    phash = new prtType*[512*512];

    clearParticles();
    clearBfr();

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
    tspecBfr = new uint8_t[1024*1024];

    initLevel(1);

    double time = 0.;

    bool leftDown = false, rightDown = false, upDown = false, downDown = false, bombDown = false;
    bool leftPressed = false, rightPressed = false, upPressed = false, downPressed = false, bombPressed = false;
    
    while (window->isOpen()) {
        leftPressed = false; rightPressed = false; upPressed = false; downPressed = false; bombPressed = false;
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == Event::Closed) {
                window->close();
            }
            else if (event.type == Event::Resized) {
	            window->setView(View(FloatRect(0.f, 0.f, (float)window->getSize().x, (float)window->getSize().y)));
            }
            else if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Key::Left || event.key.code == Keyboard::Key::A) {
                    leftDown = true;
                }
                else if (event.key.code == Keyboard::Key::Right || event.key.code == Keyboard::Key::D) {
                    rightDown = true;
                }
                if (event.key.code == Keyboard::Key::Up || event.key.code == Keyboard::Key::W) {
                    upDown = true;
                }
                else if (event.key.code == Keyboard::Key::Down || event.key.code == Keyboard::Key::S) {
                    downDown = true;
                }
                else if (event.key.code == Keyboard::Key::Space || event.key.code == Keyboard::Key::X) {
                    bombDown = true;
                }
            }
            else if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::Key::F11) {
                    fullscreen = !fullscreen;
                    delete window;
                    window = new RenderWindow(fullscreen ? VideoMode::getDesktopMode() : VideoMode(800, 600), "Lunar Oasis", fullscreen ? Style::Fullscreen : Style::Default);
                    window->setFramerateLimit(60);
	                window->setView(View(FloatRect(0.f, 0.f, (float)window->getSize().x, (float)window->getSize().y)));
                }
                else if (event.key.code == Keyboard::Key::Left || event.key.code == Keyboard::Key::A) {
                    leftDown = false;
                    leftPressed = true;
                }
                else if (event.key.code == Keyboard::Key::Right || event.key.code == Keyboard::Key::D) {
                    rightDown = false;
                    rightPressed = true;
                }
                if (event.key.code == Keyboard::Key::Up || event.key.code == Keyboard::Key::W) {
                    upDown = false;
                    upPressed = true;
                }
                else if (event.key.code == Keyboard::Key::Down || event.key.code == Keyboard::Key::S) {
                    downDown = false;
                    downPressed = true;
                }
                else if (event.key.code == Keyboard::Key::Space || event.key.code == Keyboard::Key::X) {
                    bombDown = false;
                    bombPressed = true;
                }
            }
        }

        double dt = 1. / 60.;
        time += dt;

        clearBfr();

        //

        drawSpr(LEVEL_BG[curLevel-1], 0, 0);

        if (!playerDead) {
            if (upDown) {
                float angle = (floorf(playerAngle) / 8.f) * PI * 2.f - PI * 0.5f;
                playerVX += cos(angle) * dt * PLAYER_THRUST;
                playerVY += sin(angle) * dt * PLAYER_THRUST;
            }
            if (leftDown) {
                playerAngle -= dt * PLAYER_TURN_SPEED;
            }
            if (rightDown) {
                playerAngle += dt * PLAYER_TURN_SPEED;
            }
            playerAngle = fmodf(playerAngle + 8.f * 100.f, 8.f);

            playerVX -= playerVX * dt * 0.25f;
            playerVY -= playerVY * dt * 0.25f;
            playerVY += dt * GRAVITY;
            playerX += playerVX * dt;
            playerY += playerVY * dt;
        }

        int camX = (int)round(playerX),
            camY = (int)round(playerY);

        camX = CLAMP(camX, 32, 512 - 32);
        camY = CLAMP(camY, 32, 512 - 32);

        updateRenderParticles(dt, camX, camY);

        terrainRender(camX, camY);

        for (int i=0; i<MAX_DEPOT; i++) {
            if (depots[i].exists) {
                drawSpr(DEPOT_FRAMES[CLAMP((int)(floor(depots[i].fuel * 5.f)), 0, 4)], -2 + (int)depots[i].x - camX + 32, (int)depots[i].y - camY + 32 - 2);
            }
        }

        if (flagVis) {
            drawSpr(FLAG_FRAMES[CLAMP((int)(floor(flagH * 8.f)), 0, 3)], -2 + (int)flagX - camX + 32, (int)flagY - camY + 32 - 3);
        }

        if (!playerDead) {
            bool landed = false;
            bool landingClose = (int)(floor(playerAngle)) == 0 && sprCollideTerrain(SHIP_OFF[0], (int)round(playerX) - 8, (int)round(playerY) - 8 + 3) && !upDown;
            bool justDied = false;
            if ((int)(floor(playerAngle)) == 0 && sprCollideTerrain(SHIP_OFF[0], (int)round(playerX) - 8, (int)round(playerY) - 8 + 2) && !upDown) {
                if (fabs(playerVY) > 6.f || fabs(playerVX) > 9.f) {
                    justDied = true;
                }
                else {
                    landed = true;
                }
                playerVX = 0.f;
                playerVY = 0.f;
                playerX = round(playerX);
                playerY = round(playerY);
            }
            if (sprCollideTerrain(SHIP_OFF[(int)(floor(playerAngle))], (int)round(playerX) - 8, (int)round(playerY) - 8)) {
                justDied = true;
            }


            if (upDown) {
                drawSpr(SHIP_ON[(int)(floor(playerAngle))], (int)round(playerX) - camX + 32-8, (int)round(playerY) - camY + 32-8);
                float angle = (floorf(playerAngle) / 8.f) * PI * 2.f + PI * 0.5f;
                addFire(playerX + cos(angle) * 3.5f, playerY + sin(angle) * 3.5f, cos(angle) * 20.f, sin(angle) * 20.f);
            }
            else {
                if (landed || landingClose) {
                    drawSpr(SHIP_LANDED, (int)round(playerX) - camX + 32-8, (int)round(playerY) - camY + 32-8);
                }
                else {
                    drawSpr(SHIP_OFF[(int)(floor(playerAngle))], (int)round(playerX) - camX + 32-8, (int)round(playerY) - camY + 32-8);
                }
            }

            if (justDied) {
                explosion(playerX, playerY, playerVX, playerVY, 256);
                terrainAdd(EX_BIG, (int)playerX, (int)playerY, 0, -400);
                playerDead = true;
                for (int i=0; i<MAX_DEPOT; i++) {
                    if (sqrt((playerX-depots[i].x)*(playerX-depots[i].x)+(playerY-depots[i].y)*(playerY-depots[i].y)) < 7.f) {
                        depots[i].exists = false;
                        explosion(depots[i].x, depots[i].y, 0.f, 0.f, 128);
                        terrainAdd(EX_BIG, (int)depots[i].x, (int)depots[i].y, 0, -400);
                    }
                }
                if (sqrt((playerX-flagX)*(playerX-flagX)+(playerY-flagY)*(playerY-flagY)) < 7.f) {
                    flagVis = false;
                }
            }
            else if (landed && sqrt((playerX-flagX)*(playerX-flagX)+(playerY-flagY)*(playerY-flagY)) < 7.f) {
                flagH += dt * 0.5f;
            }
            else {
                flagH -= dt * 0.5f;
                if (flagH < 0.f) {
                    flagH = 0.f;
                }
            }

            if (landed) {
                for (int i=0; i<MAX_DEPOT; i++) {
                    if (sqrt((playerX-depots[i].x)*(playerX-depots[i].x)+(playerY-depots[i].y)*(playerY-depots[i].y)) < 7.f) {
                        float take = MIN(depots[i].fuel, dt / 3.f);
                        if (take > 0.f) {
                            depots[i].fuel -= take;
                            playerFuel += take;
                            if (playerFuel > 1.f) {
                                playerFuel = 1.f;
                            }
                        }
                    }
                }
            }
        }

        if (flagH > 0.5f) {
            if (flagH > 1.f) {
                drawBox(0, 0, 64, 64, 0xFF000000);
            }
            else {
                drawNotCircle(32, 32, (int)(48.f - CLAMP((flagH*2.f - 1.f) * 48.f, 0., 48.f)), 0xFF000000);
            }
        }

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

        window->display();
    }

    delete plist;
    delete phash;
    delete terrainBfr;
    delete tspecBfr;
    delete sprBfr;
    delete spritesTex;
    delete bfr64;
    delete tex64;
    delete spr64;
    delete window;

    return 0;
}