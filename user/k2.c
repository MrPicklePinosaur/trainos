#include "usertasks.h"
#include "trainsys.h"

void
K2()
{
    Send((Tid)0x1111, (const char*)0x2222, 0x3333, (char*)0x4444, 0x5555);
}
