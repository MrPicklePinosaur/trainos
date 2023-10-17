#include <traindef.h>
#include "alloc.h"
#include "dev/uart.h"
#include "log.h"

#define MEMORY_DEBUG 0

/* dumb allocator that maintains a buffer in memory and advances ptr
 * free() calls don't do anything
 * */
typedef struct ArenaAllocator ArenaAllocator;
typedef struct AllocBlock AllocBlock;

// TODO becareful that tasks don't run into the heap
static unsigned char* const HEAP_BASE = (unsigned char*)0x00800000;
static AllocBlock* free_list = NULL;

// 1mb heap
#define ARENA_ALLOCATOR_SIZE 0x10000

typedef struct ArenaAllocator ArenaAllocator;

// TODO becareful that tasks don't run into the heap
static unsigned char* const CURSOR_ALLOC_HEAP_BASE = (unsigned char*)0x00800000;
static ArenaAllocator* alloc = (ArenaAllocator*)CURSOR_ALLOC_HEAP_BASE;

// 1mb heap
#define CURSOR_ALLOC_ALLOCATOR_SIZE 0x10000
struct ArenaAllocator {
    size_t cursor;
    unsigned int* buf[CURSOR_ALLOC_ALLOCATOR_SIZE];
};

void
cursor_alloc_init(void)
{
    *alloc = (ArenaAllocator) {
        .cursor = 0
    };
}

void*
cursor_alloc(size_t size)
{
    PRINT("alloc %d", size);
    // bounds check
    if (alloc->cursor + size >= CURSOR_ALLOC_ALLOCATOR_SIZE) {
        PANIC("arena allocator is out of memory");
    }

    void* ptr = alloc->buf + alloc->cursor;
    alloc->cursor += size;

    return ptr;
}

void
cursor_free(void* ptr)
{
    /* NOP :D */
}

struct AllocBlock {
    usize size;
    AllocBlock* next;
};

void freelist_debug(void);

void
freelist_alloc_init(void)
{

    *(AllocBlock*)(HEAP_BASE) = (AllocBlock) {
        .size = ARENA_ALLOCATOR_SIZE - sizeof(AllocBlock),
        .next = NULL,
    };

    free_list = (AllocBlock*)HEAP_BASE;
}

void*
freelist_alloc(size_t size)
{
    usize full_size = size + sizeof(AllocBlock);
    PRINT("alloc %d, full size %d", size, full_size);

    // look through free list
    AllocBlock* free_block = free_list;
    AllocBlock* prev_block = NULL;
    while (free_block != NULL) {
        
        // traverse
        if (free_block->size < size) {
            prev_block = free_block;
            free_block = free_block->next;
            continue;
        }

        // split current block if necessary
        if (free_block->size - size >= sizeof(AllocBlock)) {
            AllocBlock* next_block = (AllocBlock*)((char*)free_block + sizeof(AllocBlock) + size);
#if MEMORY_DEBUG == 1
            PRINT("next block %x", next_block);
#endif
            next_block->size = free_block->size - full_size;
            next_block->next = free_block->next;

            free_block->next = next_block;
        }

        if (prev_block != NULL) {
            prev_block->next = free_block->next;
#if MEMORY_DEBUG == 1
            PRINT("prev block %d,%x", prev_block->size, prev_block->next);
#endif
        } else {
            free_list = free_block->next;
        }

        free_block->size = size; 
        free_block->next = NULL;

#if MEMORY_DEBUG == 1
        PRINT("newly allocated block %d,%x", free_block->size, free_block->next);
        freelist_debug();
#endif

        void* ptr = (AllocBlock*)free_block + 1;
        return ptr;

    }

    // No memory left to use
    PANIC("arena allocator is out of memory");

}

void
freelist_free(void* ptr)
{
#if 0
    AllocBlock* block = (AllocBlock*)ptr - 1;

    PRINT("free %d,0x%x", block->size, block);

    // tiny sanitation check
    if (block->next != NULL) {
        LOG_WARN("Potentially corrupted malloc data structure or invalid ptr to free");
        return;
    }

    // Push to free list
    block->next = free_list;
    free_list = block;

#if MEMORY_DEBUG == 1
    freelist_debug();
#endif

    // TODO can try merging adjacent blocks to reduce fragmentization
#endif
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
