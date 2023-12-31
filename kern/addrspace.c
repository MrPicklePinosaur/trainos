#include <traindef.h>
#include <trainstd.h>

#include "addrspace.h"
#include "kern/dev/uart.h"

typedef struct PageTable PageTable;

static PageTable pagetable;

struct PageTable {
    PageTableEntry entries[MAX_PAGES]; 
};

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
            pagetable.entries[i] |= PTE_ALLOCATED;

            Address base = USER_BASE + USER_ADDRSPACE_SIZE * i;
            /* KLOG_DEBUG("base %x", (unsigned char*)base); */
            return addrspace_new(base);

        }
    }
    // TODO return error
    PANIC("Out of space in page table");
}

void
pagetable_deletepage(Addrspace* addrspace)
{
    size_t index = ((u64)addrspace->base - (u64)&pagetable)/USER_ADDRSPACE_SIZE;
    pagetable.entries[index] &= (~PTE_ALLOCATED);
}
