#include "gacha.h"
#include "log.h"

static char* const MMIO_BASE = (char*) 0xFE000000;
static char* const TIMER_BASE = (char*)(MMIO_BASE + 0x3000);
static const uint32_t TIMER_LO = 0x04;

uint32_t gacha_rand_num;

uint32_t randint(void) {
    // glibc random integer implementation
    // https://en.wikipedia.org/wiki/Linear_congruential_generator
    gacha_rand_num = (1103515245 * gacha_rand_num + 12345) % 2147483648;
    return gacha_rand_num;
}

uint32_t gacha_rarity_distribution(void) {
    uint32_t roll = randint() % 100;
    if (roll >= 33) {
        return 0;  // 67% chance of rolling a 1 star
    }
    if (roll >= 12) {
        return 1;  // 21% chance of rolling a 2 star
    }
    if (roll >= 4) {
        return 2;  // 8% chance of rolling a 3 star
    }
    if (roll >= 1) {
        return 3;  // 3% chance of rolling a 4 star
    }
    return 4;  // 1% chance of rolling a 5 star
}

void gacha_init(void) {
    gacha_rand_num = *(volatile uint32_t*)(TIMER_BASE + TIMER_LO);
}

void gacha_print_roll(void) {
    PRINT("                o}-------------------------------{o      o}------------------------------{o                ");
    PRINT("         o}------------{o   o}-------------{o                  o}-------------{o   o}-----------{o         ");
    PRINT("     o}------------{o           o}-------{o     TODAY'S ROLL     o}-------{o           o}-----------{o     ");
    PRINT("  o}-------------{o               o}-------{o                  o}-------{o               o}------------{o  ");
    PRINT("o}--------------{o                 o}------------{o      o}------------{o                 o}-------------{o");
    PRINT("");
    for (uint32_t i = 0; i < 5; i++) {
        uint32_t rarity = gacha_rarity_distribution();
        PRINT("%s  +-----------+------------------------------------------------------------------------------------------+  ", RARITY_COLORS[rarity]);
        PRINT("  |           |                                                                                          |  ");
        PRINT("  | %s |   %s |  ", RARITIES[rarity], UNITS[randint() % GACHA_UNIT_COUNT]);
        PRINT("  |           |                                                                                          |  ");
        PRINT("  +-----------+------------------------------------------------------------------------------------------+  ");
        PRINT("\x1b[0m");
    }
    PRINT("o}--------------{o                 o}----------------------------------{o                 o}-------------{o");
    PRINT("  o}-------------{o               o}------------------------------------{o               o}------------{o  ");
    PRINT("     o}------------{o           o}----------------------------------------{o           o}-----------{o     ");
    PRINT("         o}------------{o   o}------------------------------------------------{o   o}-----------{o         ");
    PRINT("                o}-----------------------------------------------------------------------{o                ");
    PRINT("");
}