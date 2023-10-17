#include <traindef.h>
#include "alloc.h"
#include "dev/uart.h"
#include "log.h"

/* dumb allocator that maintains a buffer in memory and advances ptr
 * free() calls don't do anything
 * */
typedef struct ArenaAllocator ArenaAllocator;
typedef struct AllocBlock AllocBlock;

// TODO becareful that tasks don't run into the heap
static unsigned char* const HEAP_BASE = (unsigned char*)0x00800000;
static ArenaAllocator* alloc = (ArenaAllocator*)HEAP_BASE;
static AllocBlock* free_list = NULL;

// 1mb heap
#define ARENA_ALLOCATOR_SIZE 0x10000
struct ArenaAllocator {
    usize cursor;
    unsigned int* buf[ARENA_ALLOCATOR_SIZE];
};

struct AllocBlock {
    usize size;
    AllocBlock* next;
};

void freelist_debug(void);

void
arena_init(void)
{
    *alloc = (ArenaAllocator) {
        .cursor = 0
    };

    *(AllocBlock*)(HEAP_BASE) = (AllocBlock) {
        .size = ARENA_ALLOCATOR_SIZE - sizeof(AllocBlock),
        .next = NULL,
    };

    free_list = (AllocBlock*)HEAP_BASE;
}

void*
arena_alloc(size_t size)
{
    usize full_size = size + sizeof(AllocBlock);
    PRINT("alloc %d", size);

    // look through free list
    AllocBlock* free_block = free_list;
    AllocBlock* prev_block = NULL;
    while (free_block != NULL) {
        
        // traverse
        if (free_block->size < size) {
            free_block = free_block->next;
            continue;
        }

        // split current block if necessary
        if (free_block->size - size >= sizeof(AllocBlock)) {
            AllocBlock* next_block = (AllocBlock*)((char*)free_block + sizeof(AllocBlock) + size);
            next_block->size = free_block->size - size - sizeof(AllocBlock);
            next_block->next = free_block->next;

            free_block->next = next_block;
        }

        if (prev_block != NULL) {
            prev_block->next = free_block->next;
        } else {
            free_list = free_block->next;
        }

        free_block->size = size; 
        free_block->next = NULL;

        freelist_debug();

        void* ptr = (AllocBlock*)free_block + 1;
        return ptr;

    }

    // No memory left to use
    PANIC("arena allocator is out of memory");

}

void
arena_free(void* ptr)
{
    AllocBlock* block = (AllocBlock*)ptr - 1;

    PRINT("free %d", block->size);

    // tiny sanitation check
    if (block->next != NULL) {
        LOG_WARN("Potentially corrupted malloc data structure or invalid ptr to free");
        return;
    }

    // Push to free list
    block->next = free_list;
    free_list = block;

    freelist_debug();

    // TODO can try merging adjacent blocks to reduce fragmentization
}

void
freelist_debug(void)
{
    AllocBlock* block = free_list;
    while (block != NULL) {
        uart_printf(CONSOLE, "[%x] %d,%x ", block,block->size, block->next);
        block = block->next;
    }
    uart_printf(CONSOLE, "\n");
}
