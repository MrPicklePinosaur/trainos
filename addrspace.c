#include "addrspace.h"

static PageTable pagetable;

Addrspace
addrspace_new(Address base)
{
    return (Addrspace) {
        .base = base,
        .stackbase = base + USER_ADDRSPACE_SIZE, // stack starts at bottom (high) of addrspace
    };
}

void
pagetable_init(void)
{
    pagetable = (PageTable) {
        .entries = {0}
    };
}

// Uses linear search to find the next free page
Addrspace
pagetable_createpage(void)
{
    for (unsigned int i = 0; i < MAX_PAGES; ++i) {
        if ((pagetable.entries[i] & PTE_ALLOCATED) == 0) {
            pagetable.entries[i] &= PTE_ALLOCATED;

            Address base = USER_BASE + USER_ADDRSPACE_SIZE * i;
            addrspace_new(base);

        }
    }
    // TODO return error
}
