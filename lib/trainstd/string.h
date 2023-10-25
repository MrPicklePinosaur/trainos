#ifndef __TRAINSTD_STRING_H__
#define __TRAINSTD_STRING_H__

#include <traindef.h>

// TODO (add concept of owned string? - implemented as vector)

#define str8(s) (str8){(u8*)s, lengthof(s)}
typedef struct {
    u8 *data;
    usize length;
} str8;

// allocate memory and create null terminated cstring
char* str8_to_cstr(str8 s);

str8 str8_substr(str8 s, usize start, usize end);
u8 str8_at(str8 s, usize index);
bool str8_cmp(str8 a, str8 b);

#endif // __TRAINSTD_STRING_H__
