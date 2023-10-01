#include "util.h"

// define our own memset to avoid SIMD instructions emitted from the compiler
void *memset(void *s, int c, size_t n) {
  for (char* it = (char*)s; n > 0; --n) *it++ = c;
  return s;
}

// define our own memcpy to avoid SIMD instructions emitted from the compiler
void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    char* sit = (char*)src;
    char* cdest = (char*)dest;
    for (size_t i = 0; i < n; ++i) *(cdest++) = *(sit++);
    return dest;
}

void
_panic(void)
{
    // make kernel block forever
    for (;;) {}
}

int
min(int a, int b)
{
    return (a <= b) ? a : b;
}
