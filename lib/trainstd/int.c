#include "int.h"

// saturating sub
u8
u8_sub(u8 a, u8 b)
{
    if (b > a) return 0;
    return a-b;
}

u8
u8_max(u8 a, u8 b)
{
    return (a <= b) ? b : a;
}

u8
u8_min(u8 a, u8 b)
{
    return (a <= b) ? a : b;
}

usize
usize_sub(usize a, usize b)
{
    if (b > a) return 0;
    return a-b;
}

i32
i32_abs(i32 a)
{
    if (a < 0) return -a;
    return a;
}
