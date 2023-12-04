#ifndef __TRAINSTD_INT_H__
#define __TRAINSTD_INT_H__

// utlitiy functions for standard data types

#include <traindef.h>

u8 u8_sub(u8 a, u8 b);
u8 u8_max(u8 a, u8 b);
u8 u8_min(u8 a, u8 b);

u32 u32_max(u32 a, u32 b);
u32 u32_min(u32 a, u32 b);

i32 i32_max(i32 a, i32 b);
i32 i32_min(i32 a, i32 b);
i32 i32_abs(i32 a);

usize usize_sub(usize a, usize b);

#endif // __TRAINSTD_INT_H__
