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
