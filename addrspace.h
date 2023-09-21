#ifndef __ADDRSPACE_H__
#define __ADDRSPACE_H__

#include <stdint.h>

typedef unsigned char* Address;

static unsigned char* KERN_BASE = (unsigned char*)0x00200000;
static unsigned char* USER_BASE = (unsigned char*)0x00220000;
static unsigned int USER_ADDRSPACE_SIZE = 0x00010000; // 1mb (in number of words)

#define MAX_PAGES 1024

typedef struct {
    Address base;
    Address stackbase; // the bottom of the stack
} Addrspace;

// bit mask for page table state
typedef uint16_t PageTableEntry;
#define PTE_ALLOCATED 0x1

void pagetable_init(void);

// Uses linear search to find the next free page
Addrspace pagetable_createpage(void);
void pagetable_deletepage(Addrspace* addrspace);


#endif // __ADDRSPACE_H__
