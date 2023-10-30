#include <trainstd.h>
#include "string.h"

char*
str8_to_cstr(str8 s)
{
    char* cstr = alloc((s.length+1)*sizeof(char));
    cstr = memcpy(cstr, s.data, s.length);
    cstr[s.length] = 0; // set null byte
    return cstr;
}

str8
str8_substr(str8 s, usize start, usize end)
{
    // TODO need to do validtion
    return (str8) {
        .data = s.data + start,
        .length = end-start
    };
}

u8
str8_at(str8 s, usize index)
{
    return s.data[index];
}

bool
str8_cmp(str8 a, str8 b)
{
    if (a.length != b.length) return false;
    for (u32 i = 0; i < a.length; ++i)
        if (str8_at(a, i) != str8_at(b, i)) return false;

    return true;
}

// helper for str8_format
char*
_str8_format_puts(Arena* arena, char* cur, char* buf)
{
    while (*buf) {
        *cur = *buf;
        cur = arena_alloc(arena, char);
        buf++;
    }
    return cur;
}

str8
str8_format(Arena* arena, char *fmt, ...)
{
	va_list va;
	va_start(va,fmt);

	char bf[12];
	char ch;

    char* start = arena_alloc(arena, char);
    char* cur = start;

    while ((ch = *(fmt++))) {
        if ( ch != '%' ) {
            *cur = ch;
            cur = arena_alloc(arena, char);
            continue;
        }
        ch = *(fmt++);
        switch( ch ) {
            case 'u':
                ui2a( va_arg( va, unsigned int ), 10, bf );
                cur = _str8_format_puts(arena, cur, bf);
                break;
            case 'd':
                i2a( va_arg( va, int ), bf );
                cur = _str8_format_puts(arena, cur, bf);
                break;
            case 'x':
                ui2a( va_arg( va, unsigned int ), 16, bf );
                cur = _str8_format_puts(arena, cur, bf);
                break;
            case 's':
                cur =_str8_format_puts(arena, cur, va_arg( va, char* ));
                break;
            case 'c':
                *cur = va_arg( va, int );
                cur = arena_alloc(arena, char);
                break;
            case '%':
                *cur = ch;
                cur = arena_alloc(arena, char);
                break;
            case '\0':
                break;
        }
    }

	va_end(va);

    return str8(start);
}
