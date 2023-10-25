#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/nameserver.h"

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

    Tid task1 = Create(1, &testingTask1, "testingTask1");
    Tid task2 = Create(1, &testingTask2, "testingTask2");
    Yield();

    // TODO we need to wait for task1 and task2 to run, so sorta sus

    TEST((Tid)WhoIs("firstTask") == task1);
    TEST((Tid)WhoIs("secondTask") == task2);
    TEST((Tid)WhoIs("thirdTask") == 0); // should be not found

    Exit();
}