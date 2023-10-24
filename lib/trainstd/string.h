#ifndef __TRAINSTD_STRING_H__
#define __TRAINSTD_STRING_H__

#include <traindef.h>

#define str8(s) (str8){(u8*)s, lengthof(s)}
typedef struct {
    u8 *data;
    usize length;
} str8;

#endif // __TRAINSTD_STRING_H__
