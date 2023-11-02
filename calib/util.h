#ifndef _util_h_
#define _util_h_ 1

#include <stddef.h>
#include <stdint.h>

#define sizeof(x)    (size_t)sizeof(x)
#define alignof(x)   (size_t)_Alignof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(s)  (countof(s) - 1)

// conversions
int a2d(char ch);
char a2i( char ch, char **src, int base, int *nump );
void ui2a( unsigned int num, unsigned int base, char *bf );
void i2a( int num, char *bf );

void delay(uint32_t amount);

// memory
void *memset(void *s, int c, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);

#endif /* util.h */
