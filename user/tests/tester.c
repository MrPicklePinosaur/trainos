#include "user/usertasks.h"
#include "user/nameserver.h"
#include "tester.h"
#include <trainsys.h>
#include <trainstd.h>
#include <stdbool.h>

/* user tasks for running userland tests such as data structures */

void
testHarness()
{
    TestFn tests[] = {
        &testSensor,
        &testString,
        &testCbuf,
        &testList,
        &testHashmap,
        &testMap,
        &testAlloc,
        &testParser,
        /* &testNameserver, */
        0
    };

    for (usize i = 0; tests[i] != 0; ++i) {
        Tid test_task = Create(1, tests[i], "testRunner");
        WaitTid(test_task);
    }

    Exit();
}
