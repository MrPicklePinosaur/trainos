#include "usertasks.h"
#include <trainsys.h>
#include <trainstd.h>

void
awaitEventTest()
{
    for (;;) {
        AwaitEvent(EVENT_CLOCK_TICK);
        println("got clock tick event");
    }
}


void
K3()
{
    Exit();
}
