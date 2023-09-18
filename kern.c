#include "kern.h"
#include "addrspace.h"
#include "task.h"

void
kern_init(void)
{
    pagetable_init();
    tasktable_init();
}
