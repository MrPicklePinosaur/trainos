#ifndef __TRAINDEF_H__
#define __TRAINDEF_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef ptrdiff_t isize;
typedef size_t usize;

typedef char byte;

#define sizeof(x)    (usize)sizeof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(s)  (countof(s) - 1)

#endif // __TRAINDEF_H__
