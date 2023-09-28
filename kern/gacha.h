#ifndef __GACHA_H__
#define __GACHA_H__

#include <stdint.h>

#define GACHA_UNIT_COUNT 15
#define GACHA_RARITY_COUNT 5

static const char* const UNITS[GACHA_UNIT_COUNT] = {
    "MATH 135   \"...if there exists b in Z such that 2b = a...\"                            ",
    "MATH 136   \"...if there exists a scalar lambda such that Ax = lambda x...\"            ",
    "MATH 137   \"...for any epsilon, there exists a delta such that...\"                    ",
    "MATH 138   \"...int f(x)g'(x) dx = f(x)g(x) - int f'(x)g(x) dx...\"                     ",
    "MATH 239   \"...each face has degree at least d*, then (d* - 2)q <= d*(p - 2)...\"      ",
    "CS 135     \"...(define (fib n) (cond ((= n 0) 0) ((= n 1) 1) ((> n 1) (...\"           ",
    "CS 136     \"...struct Node {int data; struct Node *next;}; struct Node *make_node...\" ",
    "CS 145     \"...(define Y (lambda (f) ((lambda (self) (f (self self))) (lambda...\"     ",
    "CS 146     \"...struct FRAST interp(struct FRAST expr) { if (expr.type == APP)...\"     ",
    "CS 240     \"...decoding is O(|C|). This is optimal; it is the shortest prefix-free...\"",
    "CS 241     \"...reg.link = savedPC; Reg.framePointer = dynamicLink; Stack.pop...\"      ",
    "CS 245     \"...sigma, A |- B is a theorem, then sigma |- A -> B is a theorem...\"      ",
    "CS 251     \"...LDUR X1, [X2, #20]; NOP; NOP; CBZ X1, #4...\"                           ",
    "CS 370     \"...f(t) = a0 + sum a_i cos (2 pi k t/T) + sum b_i sin (2 pi k t / T)...\"  ",
    "STAT 230   \"...probability function of f(x) = e^-u * u^x / x! where u > 0 is...\"      ",
};
static const char* const RARITIES[GACHA_RARITY_COUNT] = {
    "    o    ",
    "   o o   ",
    "  o o o  ",
    " o o o o ",
    "o o o o o"
};
static const char* const RARITY_COLORS[GACHA_RARITY_COUNT] = {
    "\x1b[0m",
    "\x1b[38;5;75m",
    "\x1b[38;5;186m",
    "\x1b[38;5;176m",
    "\x1b[38;5;46m",
};

uint32_t randint(void);

void gacha_init(void);
uint32_t gacha_rarity_distribution(void);
void gacha_print_roll(void);

#endif  // __GACHA_H__