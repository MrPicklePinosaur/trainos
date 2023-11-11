#include "usertasks.h"
#include <trainsys.h>
#include <trainstd.h>

void
firstUserTask()
{
    Tid t1 = Create(5, &secondUserTask, "K1 User Task (Priority 5)");
    println("Created: %d", t1);
    Tid t2 = Create(5, &secondUserTask, "K1 User Task (Priority 5)");
    println("Created: %d", t2);

    Tid t3 = Create(3, &secondUserTask, "K1 User Task (Priority 3)");
    println("Created: %d", t3);
    Tid t4 = Create(3, &secondUserTask, "K1 User Task (Priority 3)");
    println("Created: %d", t4);

    println("FirstUserTask: exiting");
    Exit();
}

void
secondUserTask()
{
    println("MyTid = %d, MyParentTid = %d", MyTid(), MyParentTid());
    Yield();
    println("MyTid = %d, MyParentTid = %d", MyTid(), MyParentTid());
    Exit();
}
