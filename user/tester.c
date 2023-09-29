#include "usertasks.h"
#include "trainsys.h"

#include "trainstd.h"

/* user tasks for running userland tests such as data structures */

#define assert(expr) { if (expr) { println("\033[32m[PASSED]\033[0m "#expr); } else { println("\033[31m[FAILED]\033[0m "#expr); } }


void testCbuf();

void
testHarness()
{
    Create(1, &testCbuf);
    Yield();

    Exit();
}

void
testCbuf()
{
    println("Running test suite for cbuf -----------------");

    CBuf* out_stream = cbuf_new(10);
    assert(cbuf_len(out_stream) == 0);

    cbuf_push_front(out_stream, 0x1);
    assert(cbuf_len(out_stream) == 1);

    cbuf_push_front(out_stream, 0x2);
    cbuf_push_front(out_stream, 0x3);
    assert(cbuf_len(out_stream) == 3);

    uint8_t val = cbuf_pop_front(out_stream);
    assert(cbuf_len(out_stream) == 2);
    assert(val == 0x3);

    cbuf_push_back(out_stream, 0x42);
    assert(cbuf_len(out_stream) == 3);
    assert(cbuf_back(out_stream) == 0x42);

    val = cbuf_pop_back(out_stream);
    assert(cbuf_len(out_stream) == 2);
    assert(val == 0x42);

    cbuf_clear(out_stream);
    assert(cbuf_len(out_stream) == 0);

    cbuf_push_back(out_stream, 0);
    cbuf_push_back(out_stream, 1);
    cbuf_push_back(out_stream, 2);
    cbuf_push_back(out_stream, 3);

    assert(cbuf_len(out_stream) == 4);
    assert(cbuf_get(out_stream, 0) == 0);
    assert(cbuf_get(out_stream, 1) == 1);
    assert(cbuf_get(out_stream, 2) == 2);
    assert(cbuf_get(out_stream, 3) == 3);

    Exit();
}
