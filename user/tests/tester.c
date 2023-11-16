#include <trainsys.h>
#include <trainstd.h>
#include <traintasks.h>
#include "user/usertasks.h"
#include "tester.h"

/* user tasks for running userland tests such as data structures */

void
testHarness()
{
    TestFn tests[] = {
        /* &testSensor, */
        /* &testDijkstra, */
        //&testString,
        &testMap,
        &testCbuf,
        &testList,
        &testHashmap,
        &testAlloc,
        &testParser,
        /* &testNameserver, */
        0
    };

    for (usize i = 0; tests[i] != 0; ++i) {
        Tid test_task = Create(1, tests[i], "Test Runner");
        WaitTid(test_task);
    }

    Exit();
}
