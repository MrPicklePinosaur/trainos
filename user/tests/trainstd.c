#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"

// test standard library

void
testString()
{
    println("Running test suite for string -----------------");

    Arena arena = arena_new(128);

    PRINT("%s", str8("hello world"));
    PRINT("%s", str8_format(&arena, "abcd"));
    PRINT("%s", str8_format(&arena, "uint = %u, int = %d, hex = %x, str = %s, char = %c ", 128, -10, 0x69, "locomotive", 'A'));
    
    Exit();
}

void
testCbuf()
{
    println("Running test suite for cbuf -----------------");

    CBuf* out_stream = cbuf_new(10);
    TEST(cbuf_len(out_stream) == 0);

    cbuf_push_front(out_stream, 0x1);
    TEST(cbuf_len(out_stream) == 1);

    cbuf_push_front(out_stream, 0x2);
    cbuf_push_front(out_stream, 0x3);
    TEST(cbuf_len(out_stream) == 3);

    u8 val = cbuf_pop_front(out_stream);
    TEST(cbuf_len(out_stream) == 2);
    TEST(val == 0x3);

    cbuf_push_back(out_stream, 0x42);
    TEST(cbuf_len(out_stream) == 3);
    TEST(cbuf_back(out_stream) == 0x42);

    val = cbuf_pop_back(out_stream);
    TEST(cbuf_len(out_stream) == 2);
    TEST(val == 0x42);

    cbuf_clear(out_stream);
    TEST(cbuf_len(out_stream) == 0);

    cbuf_push_back(out_stream, 0);
    cbuf_push_back(out_stream, 1);
    cbuf_push_back(out_stream, 2);
    cbuf_push_back(out_stream, 3);

    TEST(cbuf_len(out_stream) == 4);
    TEST(cbuf_get(out_stream, 0) == 0);
    TEST(cbuf_get(out_stream, 1) == 1);
    TEST(cbuf_get(out_stream, 2) == 2);
    TEST(cbuf_get(out_stream, 3) == 3);

    CBuf* cbuf2 = cbuf_new(10);
    cbuf_push_front(cbuf2, 3);
    cbuf_push_front(cbuf2, 2);
    cbuf_push_front(cbuf2, 1);
    cbuf_push_front(cbuf2, 0);

    for (int i = 0; i < 4; ++i) {
        TEST(cbuf_get(cbuf2, i) == i);
    }

    Exit();
}

void
testList()
{
    println("Running test suite for list -----------------");
    List* list = list_init();
    TEST(list_len(list) == 0);

    list_push_back(list, (void*)2);
    TEST(list_peek_front(list) == (void*)2);  
    TEST(list_peek_back(list) == (void*)2);  
    TEST(list_len(list) == 1);

    list_push_front(list, (void*)1);
    TEST(list_peek_front(list) == (void*)1);
    TEST(list_peek_back(list) == (void*)2);  
    TEST(list_len(list) == 2);

    list_pop_front(list);
    TEST(list_peek_front(list) == (void*)2);  
    TEST(list_peek_back(list) == (void*)2);  
    TEST(list_len(list) == 1);

    // second test, remove a given element
    List* list2 = list_init();
    for (usize i = 0; i < 10; ++i) list_push_back(list2, (void*)i);
    TEST(list_peek_front(list2) == (void*)0);  
    TEST(list_peek_back(list2) == (void*)9);  
    TEST(list_len(list2) == 10);

    TEST(list_remove(list2, (void*)7) == true);

    ListIter it2 = list_iter(list2);
    void* item;
    usize index = 0;
    while (listiter_next(&it2, &item)) {
        println("item %d, index %d", item, index);
        TEST(item == index);
        ++index;
        if (index == 7) ++index;
    }
    TEST(list_peek_front(list2) == (void*)0);
    TEST(list_peek_back(list2) == (void*)9);
    TEST(list_len(list2) == 9);


    Exit();
}

void
testMap()
{
    println("Running test suite for map -----------------");
    Arena arena = arena_new(256);

    Map* map = NULL;
    map_insert(&map, str8("A10"), (mapval_t)69, &arena);
    map_insert(&map, str8("A11"), (mapval_t)6969, &arena);
    map_insert(&map, str8("A12"), (mapval_t)696969, &arena);

    TEST(map_contains(&map, str8("A10"), &arena) == true);
    TEST(map_contains(&map, str8("A11"), &arena) == true);
    TEST(map_contains(&map, str8("A12"), &arena) == true);

    TEST(map_get(&map, str8("A10")) == (mapval_t)69);
    TEST(map_get(&map, str8("A11")) == (mapval_t)6969);
    TEST(map_get(&map, str8("A12")) == (mapval_t)696969);

    map_insert(&map, str8("A10"), (mapval_t)21, &arena);
    TEST(map_contains(&map, str8("A10"), &arena) == true);
    TEST(map_get(&map, str8("A10")) == (mapval_t)21);

    Exit();
}

void
testHashmap()
{
    println("Running test suite for hashmap -----------------");
    HashMap* map = hashmap_new(20);
    TEST(hashmap_size(map) == 0);

    hashmap_insert(map, "one", (void*)1);
    hashmap_insert(map, "two", (void*)2);
    hashmap_insert(map, "three", (void*)3);
    hashmap_insert(map, "four", (void*)4);
    TEST(hashmap_size(map) == 4);

    bool success;
    TEST(hashmap_get(map, "one", &success) == (void*)1);
    TEST(success);
    TEST(hashmap_get(map, "two", &success) == (void*)2);
    TEST(success);
    TEST(hashmap_get(map, "five", &success) == (void*)0);
    TEST(!success);

    TEST(hashmap_contains(map, "one") == true);
    TEST(hashmap_contains(map, "four") == true);
    TEST(hashmap_contains(map, "five") == false);

    TEST(hashmap_remove(map, "one") == true);
    TEST(hashmap_remove(map, "three") == true);
    TEST(hashmap_remove(map, "four") == true);
    TEST(hashmap_remove(map, "one") == false);
    TEST(hashmap_remove(map, "five") == false);
    TEST(hashmap_size(map) == 1);

    TEST(hashmap_get(map, "one", &success) == (void*)0);
    TEST(!success);
    TEST(hashmap_get(map, "two", &success) == (void*)2);
    TEST(success);
    TEST(hashmap_get(map, "three", &success) == (void*)0);
    TEST(!success);
    TEST(hashmap_get(map, "five", &success) == (void*)0);
    TEST(!success);

    Exit();
}

void
testAlloc()
{
    println("Running test suite for memory allocator -----------------");

    void* ptrs[50] = {0};
    ptrs[0] = alloc(10);
    ptrs[1] = alloc(12);
    ptrs[2] = alloc(14);
    free(ptrs[1]);
    free(ptrs[0]);
    ptrs[3] = alloc(40);
    free(ptrs[3]);
    ptrs[4] = alloc(8);
    free(ptrs[2]);

    for (int i = 0; i < 50; ++i) {
        ptrs[i] = alloc(i % 7 + 1);
    }
    for (int i = 0; i < 50; ++i) {
        free(ptrs[49 - i]);
    }

    println("done");

    Exit();
}
