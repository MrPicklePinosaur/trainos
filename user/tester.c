#include "usertasks.h"
#include "trainsys.h"

#include "trainstd.h"

/* user tasks for running userland tests such as data structures */

#define assert(expr) { if (expr) { println("\033[32m[PASSED]\033[0m"); } else { println("\033[31m[FAILED]\033[0m "#expr); } }


void testCbuf();

void
testHarness()
{
    Create(1, &testCbuf);

    Exit();
}

void
testCbuf()
{
    CBuf* out_stream = cbuf_new(10);
    assert(cbuf_len(out_stream) == 0);
    cbuf_push(out_stream, 0x1);
    assert(cbuf_len(out_stream) == 1);
    cbuf_push(out_stream, 0x2);
    cbuf_push(out_stream, 0x3);
    assert(cbuf_len(out_stream) == 3);
    uint8_t val = cbuf_pop(out_stream);
    assert(cbuf_len(out_stream) == 2);
    assert(val == 0x3);

    /* println("%u, val = %u", cbuf_len(out_stream), val); */
    Exit();
}
