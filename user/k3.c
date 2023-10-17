#include "usertasks.h"
#include <trainsys.h>
#include <trainstd.h>

void
K3() {
    for (;;) {
        AwaitEvent(EVENT_CLOCK_TICK);
        println("got clock tick event");
    }
}

