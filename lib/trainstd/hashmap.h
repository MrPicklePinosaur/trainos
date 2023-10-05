#ifndef __TRAINSTD_HASHMAP_H__
#define __TRAINSTD_HASHMAP_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct HashMap HashMap;
typedef char* key_t;
typedef void* value_t;

HashMap* hashmap_new(size_t bucket_count);
void hashmap_delete(HashMap* hm);
int hashmap_insert(HashMap* hm, key_t key, value_t value);
bool hashmap_contains(HashMap* hm, key_t key);
bool hashmap_remove(HashMap* hm, key_t key);
value_t hashmap_get(HashMap* hm, key_t key, bool* success);
size_t hashmap_size(HashMap* hm);

#endif // __TRAINSTD_HASHMAP_H__
