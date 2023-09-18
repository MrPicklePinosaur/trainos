#ifndef __ADDRSPACE_H__
#define __ADDRSPACE_H__

#include <stdint.h>

typedef uint64_t Address;

static const Address KERN_BASE = 0x00200000;
static const Address USER_BASE = 0x01000000;
static const uint64_t USER_ADDRSPACE_SIZE = 0x01000000; // 1mb

#define MAX_PAGES 1024

/* typedef struct { */
/*     /1* list of all ARM registers we need to save *1/ */
/* } Trapframe; */

typedef struct {
    Address base;
    Address stackbase;
} Addrspace;

// bit mask for page table state
typedef uint16_t PageTableEntry;
#define PTE_ALLOCATED 0x1

typedef struct {
    PageTableEntry entries[MAX_PAGES]; 
} PageTable;

void pagetable_init(void);

// Uses linear search to find the next free page
Addrspace pagetable_createpage(void);


#endif // __ADDRSPACE_H__
