#include "int.h"

// saturating sub
u8
u8_sub(u8 a, u8 b)
{
    if (b > a) return 0;
    return a-b;
}

usize
usize_sub(usize a, usize b)
{
    if (b > a) return 0;
    return a-b;
}
