#include "mem.h"
#include "hashmap.h"
#include "list.h"
#include "pair.h"
#include <string.h>
#include <trainstd.h>

typedef PAIR(key_t, value_t) HashMapPair;

struct HashMap {
    List** buckets;
    size_t bucket_count;
    size_t size;
};

size_t
hashfunction(HashMap* hm, key_t key)
{
    // hash function is when we sum up ascii values of string and normalize to table size
    size_t res = 0;
    for (unsigned int i = 0; i < strlen(key); ++i) {
        res += key[i]; 
    }
    res = res % hm->bucket_count;
    return res;
}

HashMap*
hashmap_new(size_t bucket_count)
{
    List** buckets = alloc(sizeof(List*)*bucket_count);
    for (unsigned int i = 0; i < bucket_count; ++i) {
        buckets[i] = list_init();
    }

    HashMap* map = alloc(sizeof(HashMap));
    *map = (HashMap) {
        .buckets = buckets,
        .bucket_count = bucket_count,
        .size = 0
    };
    return map;
}

void
hashmap_delete(HashMap* hm)
{
    // we don't free items inside hashmap?
    // TODO also free each pair inside bucket
    for (unsigned int i = 0; i < hm->bucket_count; ++i) list_deinit(hm->buckets[i]);

    free(hm->buckets);
    free(hm);
}

int
hashmap_insert(HashMap* hm, key_t key, value_t value)
{
    size_t ind = hashfunction(hm, key);

    // TODO: we actually need a linked list of pairs
    HashMapPair* pair = alloc(sizeof(HashMapPair));
    *pair = (HashMapPair) {
        .first = key,
        .second = value,
    };

    list_push_back(hm->buckets[ind], pair);

    ++(hm->size);

    return 0;
}

bool
hashmap_contains(HashMap* hm, key_t key)
{
    size_t ind = hashfunction(hm, key);

    List* bucket = hm->buckets[ind];
    ListIter* it = list_iter(bucket);
    HashMapPair* pair;
    while((pair = (HashMapPair*)listiter_next(it)) != 0) {
        if (strcmp(pair->first, key) == 0) return true;
    }
    return false;
}

value_t
hashmap_get(HashMap* hm, key_t key, bool* success)
{
    size_t ind = hashfunction(hm, key);

    List* bucket = hm->buckets[ind];
    ListIter* it = list_iter(bucket);
    HashMapPair* pair;
    while((pair = (HashMapPair*)listiter_next(it)) != 0) {
        if (strcmp(pair->first, key) == 0) {
            *success = true;
            return pair->second;
        }
    }

    *success = false;
    return 0;
}

size_t
hashmap_size(HashMap* hm)
{
    return hm->size;
}
