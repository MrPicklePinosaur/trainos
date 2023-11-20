#include "rand.h"
#include "kern/dev/timer.h"

u32 rand_num;

void
rand_init(void)
{
    rand_num = timer_get();
}

u32
rand_int(void)
{
    // glibc random integer implementation
    // https://en.wikipedia.org/wiki/Linear_congruential_generator
    rand_num = (1103515245 * rand_num + 12345) % 2147483648;
    return rand_num;
}
