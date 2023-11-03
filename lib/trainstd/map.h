#ifndef __TRAINSTD_MAP_H__
#define __TRAINSTD_MAP_H__

#include <trainstd.h>

// arena backed hashmap implementation
// https://nullprogram.com/blog/2023/09/30/


typedef str8 mapkey_t;
typedef void* mapval_t;

typedef struct Map Map;
struct Map {
    Map* child[4];
    mapkey_t  key;
    mapval_t  value;
};

mapval_t* insert(Map** m, mapkey_t key, Arena* arena);

#endif // __TRAINSTD_MAP_H__
