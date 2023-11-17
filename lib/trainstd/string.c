#include <trainstd.h>
#include "string.h"


char* _cstr_format_puts(Arena* arena, char* cur, char* buf);
char* _cstr_format(Arena* arena, char *fmt, va_list va);

char*
str8_to_cstr(str8 s)
{
    return (char*)s.data;
}

str8
str8_from_cstr(char* cstr)
{
    return (str8) {
        .data = (u8*)cstr,
        .length = cstr_len(cstr)
    };
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

usize
str8_len(str8 s)
{
    return s.length;
}

bool
str8_cmp(str8 a, str8 b)
{
    if (a.length != b.length) return false;
    for (u32 i = 0; i < a.length; ++i)
        if (str8_at(a, i) != str8_at(b, i)) return false;

    return true;
}

// copies data inside string and returns a new string
str8
str8_copy(Arena* arena, str8 s)
{
    char* buf = arena_alloc(arena, char, str8_len(s)+1);
    for (usize i = 0; i < str8_len(s); ++i) {
        buf[i] = str8_at(s, i);
    } 
    buf[str8_len(s)] = 0; // null byte
    return (str8) {
        .data = buf,
        .length = str8_len(s)
    };
}

str8
str8_format(Arena* arena, char *fmt, ...)
{
	va_list va;
	va_start(va,fmt);
    char* out = _cstr_format(arena, fmt, va);
	va_end(va);
    return str8_from_cstr(out);
}

// TODO: assumes that string consists only of numeric characters
// also doesn't support negative numbers
u64
str8_to_u64(str8 s)
{
    u64 out = 0;
    for (usize i = 0; i < str8_len(s); ++i)
        out = 10*out + (str8_at(s, i)-'0');
    return out;
}

// copy with the null terminator
char*
cstr_copy(Arena* arena, char* s)
{
    usize len_str = cstr_len(s);
    char* new_str = arena_alloc(arena, char, len_str+1);
    memcpy(new_str, s, len_str);
    new_str[len_str] = 0;
    return new_str;
}

// helper for cstr_format
char*
_cstr_format_puts(Arena* arena, char* cur, char* buf)
{
    while (*buf) {
        *cur = *buf;
        cur = arena_alloc(arena, char);
        buf++;
    }
    return cur;
}

char*
_cstr_format(Arena* arena, char *fmt, va_list va)
{
	char ch;

    char* start = arena_alloc(arena, char);
    char* cur = start;

    for (;;) {
        ch = *(fmt++);
        if (ch == 0) {
            *cur = 0; // add null terminator
            break;
        }
        if ( ch != '%' ) {
            *cur = ch;
            cur = arena_alloc(arena, char);
            continue;
        }
        ch = *(fmt++);
        char bf[12] = {0};
        switch( ch ) {
            case 'u':
                ui2a( va_arg( va, unsigned int ), 10, bf );
                cur = _cstr_format_puts(arena, cur, bf);
                break;
            case 'd':
                i2a( va_arg( va, int ), bf );
                cur = _cstr_format_puts(arena, cur, bf);
                break;
            case 'x':
                ui2a( va_arg( va, unsigned int ), 16, bf );
                cur = _cstr_format_puts(arena, cur, bf);
                break;
            case 's':
                cur = _cstr_format_puts(arena, cur, va_arg( va, char* ));
                break;
            case 'c':
                *cur = (char)va_arg( va, int );
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

    return start;
}

char*
cstr_format(Arena* arena, char *fmt, ...)
{
	va_list va;
	va_start(va,fmt);
    char* out = _cstr_format(arena, fmt, va);
	va_end(va);
    return out;
}

u64
cstr_to_u64(char* str)
{

}

#define CSTR_LEN_MAX_LENGTH 256
usize
cstr_len(char* s)
{
    // TODO put some safety mechanism to prevent executing for very long if missing null terminator?
    usize len = 0;
    for (; *s != 0; ++s) {
        if (len > CSTR_LEN_MAX_LENGTH) {
            PANIC("Potenitally missing null terminator in string");
        }
        ++len;
    }
    return len;
}
