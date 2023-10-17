#include "usertasks.h"
#include "nameserver.h"
#include <trainsys.h>
#include <trainstd.h>
#include <stdbool.h>

/* user tasks for running userland tests such as data structures */

#define TEST(expr) { if (expr) { println("\033[32m[PASSED]\033[0m "#expr); } else { println("\033[31m[FAILED]\033[0m "#expr); } }


void testCbuf();
void testHashmap();
void testNameserver();
void testAlloc();

void
testHarness()
{
    Create(1, &testCbuf);
    Yield();
    Create(1, &testHashmap);
    Yield();
    Create(5, &testNameserver);
    Yield();
    Create(1, &testAlloc);
    Yield();

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
testingTask1()
{
    TEST(RegisterAs("firstTask") == 0);
    Yield();
    Exit();
}

void
testingTask2()
{
    TEST(RegisterAs("secondTask") == 0);
    Yield();
    Exit();
}

void
testNameserver()
{
    println("Running test suite for nameserver -----------------");

    Tid task1 = Create(1, &testingTask1);
    Tid task2 = Create(1, &testingTask2);
    Yield();

    // TODO we need to wait for task1 and task2 to run, so sorta sus

    TEST((Tid)WhoIs("firstTask") == task1);
    TEST((Tid)WhoIs("secondTask") == task2);
    TEST((Tid)WhoIs("thirdTask") == 0); // should be not found

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
