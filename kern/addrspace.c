#include <stddef.h>

#include "addrspace.h"
#include "uart.h"
#include "log.h"

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
            /* LOG_DEBUG("base %x", (unsigned char*)base); */
            return addrspace_new(base);

        }
    }
    // TODO return error
    LOG_ERROR("Out of space in page table");
}

void
pagetable_deletepage(Addrspace* addrspace)
{
    size_t index = ((uint64_t)addrspace->base - (uint64_t)&pagetable)/USER_ADDRSPACE_SIZE;
    pagetable.entries[index] &= (~PTE_ALLOCATED);
}
