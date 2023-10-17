#include <traindef.h>
#include "gacha.h"
#include "log.h"
#include "kern/dev/timer.h"

u32 gacha_rand_num;

u32
randint(void)
{
    // glibc random integer implementation
    // https://en.wikipedia.org/wiki/Linear_congruential_generator
    gacha_rand_num = (1103515245 * gacha_rand_num + 12345) % 2147483648;
    return gacha_rand_num;
}

u32
gacha_rarity_distribution(void)
{
    u32 roll = randint() % 100;
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

void
gacha_init(void)
{
    gacha_rand_num = timer_get();
}

void
gacha_print_roll(void)
{
    PRINT("                o}-------------------------------{o      o}------------------------------{o                ");
    PRINT("         o}------------{o   o}-------------{o                  o}-------------{o   o}-----------{o         ");
    PRINT("     o}------------{o           o}-------{o     TODAY'S ROLL     o}-------{o           o}-----------{o     ");
    PRINT("  o}-------------{o               o}-------{o                  o}-------{o               o}------------{o  ");
    PRINT("o}--------------{o                 o}------------{o      o}------------{o                 o}-------------{o");
    PRINT("");
    for (u32 i = 0; i < 5; i++) {
        u32 rarity = gacha_rarity_distribution();
        PRINT("%s  +-----------+------------------------------------------------------------------------------------------+  ", RARITY_COLORS[rarity]);
        PRINT("%s  |           |                                                                                          |  ", RARITY_COLORS[rarity]);
        PRINT("%s  | %s |   %s |  ", RARITY_COLORS[rarity], RARITIES[rarity], UNITS[randint() % GACHA_UNIT_COUNT]);
        PRINT("%s  |           |                                                                                          |  ", RARITY_COLORS[rarity]);
        PRINT("%s  +-----------+------------------------------------------------------------------------------------------+  ", RARITY_COLORS[rarity]);
        PRINT("\x1b[0m");
    }
    PRINT("o}--------------{o                 o}----------------------------------{o                 o}-------------{o");
    PRINT("  o}-------------{o               o}------------------------------------{o               o}------------{o  ");
    PRINT("     o}------------{o           o}----------------------------------------{o           o}-----------{o     ");
    PRINT("         o}------------{o   o}------------------------------------------------{o   o}-----------{o         ");
    PRINT("                o}-----------------------------------------------------------------------{o                ");
    PRINT("");
}
