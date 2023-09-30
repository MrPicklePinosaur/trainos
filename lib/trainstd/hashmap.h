#ifndef __TRAINSTD_HASHMAP_H__
#define __TRAINSTD_HASHMAP_H__

/* TODO: implementation is WIP, don't use */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct HashMap HashMap;
typedef uint32_t hash_t;
typedef void* value_t;

HashMap* hashmap_new(size_t bucket_count);
void hashmap_delete(HashMap* hm);
int hashmap_insert(HashMap* hm, hash_t hash, value_t value);
bool hashmap_contains(HashMap* hm, hash_t hash);
value_t hashmap_get(HashMap* hm, hash_t hash, int* success);

#endif // __TRAINSTD_HASHMAP_H__
