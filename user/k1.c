#include "k1.h"
#include "trainsys.h"
#include "trainstd.h"

void
firstUserTask()
{
    Tid t1 = Create(5, &secondUserTask);
    println("Created: %d", t1);
    Tid t2 = Create(5, &secondUserTask);
    println("Created: %d", t2);

    Tid t3 = Create(3, &secondUserTask);
    println("Created: %d", t3);
    Tid t4 = Create(3, &secondUserTask);
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
