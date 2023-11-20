#include <traindef.h>
#include <trainstd.h>
#include "gacha.h"

u32
gacha_rarity_distribution(void)
{
    u32 roll = rand_int() % 100;
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
        PRINT("%s  | %s |   %s |  ", RARITY_COLORS[rarity], RARITIES[rarity], UNITS[rand_int() % GACHA_UNIT_COUNT]);
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
