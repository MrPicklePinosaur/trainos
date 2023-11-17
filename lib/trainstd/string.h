#ifndef __TRAINSTD_STRING_H__
#define __TRAINSTD_STRING_H__

#include <traindef.h>
#include <stdarg.h>
#include <string.h>
#include "arena.h"

// TODO (add concept of owned string? - implemented as vector)

#define str8(s) (str8){(u8*)s, lengthof(s)}
typedef struct {
    u8 *data;
    usize length;
} str8;

char* str8_to_cstr(str8 s);
str8 str8_from_cstr(char* cstr);

str8 str8_substr(str8 s, usize start, usize end);
u8 str8_at(str8 s, usize index);
usize str8_len(str8 s);
bool str8_cmp(str8 a, str8 b);
str8 str8_copy(Arena* arena, str8 s);
str8 str8_replicate(Arena* arena, char ch, usize times);
str8 str8_concat(str8 a, str8 b);
str8 str8_format(Arena* arena, char *fmt, ...);
u64 str8_to_u64(str8 s);

// cstring functions that work with allocators
char* cstr_copy(Arena* arena, char* s);
char* cstr_format(Arena* arena, char *fmt, ...);
char* _cstr_format(Arena* arena, char *fmt, va_list va);
usize cstr_len(char* s);

#endif // __TRAINSTD_STRING_H__
