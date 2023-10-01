#include "usertasks.h"
#include <trainsys.h>
#include <trainstd.h>
#include <stdbool.h>

/* user tasks for running userland tests such as data structures */

#define TEST(expr) { if (expr) { println("\033[32m[PASSED]\033[0m "#expr); } else { println("\033[31m[FAILED]\033[0m "#expr); } }


void testCbuf();
void testHashmap();
void testNameserver();

void
testHarness()
{
    Create(1, &testCbuf);
    Yield();
    Create(1, &testHashmap);
    Yield();
    Create(5, &testNameserver);
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

    uint8_t val = cbuf_pop_front(out_stream);
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

    hashmap_insert(map, "one", 1);
    hashmap_insert(map, "two", 2);
    hashmap_insert(map, "three", 3);
    hashmap_insert(map, "four", 4);
    TEST(hashmap_size(map) == 4);

    bool success;
    TEST(hashmap_get(map, "one", &success) == 1);
    TEST(success);
    TEST(hashmap_get(map, "two", &success) == 2);
    TEST(success);
    TEST(hashmap_get(map, "five", &success) == 0);
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

    TEST(WhoIs("firstTask") == task1);
    TEST(WhoIs("secondTask") == task2);
    TEST(WhoIs("thirdTask") == 0); // should be not found

    Exit();
}
