#include "fs.h"
#include "task.h"
#include "alloc.h"

typedef struct Superblock Superblock;
typedef struct FileOpenTable FileOpenTable;
typedef struct FileOpenTableEntry FileOpenTableEntry;

static Superblock superblock;
static FileOpenTable open_table;

// Current implementation: Files have a fixed max size and file id (FID) is just an index into where the file is stored.

struct Superblock {
    // map file names to a location (inode)
    FileMeta* meta_list[FS_BLOCK_COUNT];
};


struct FileOpenTableEntry {
    Tid tid; // the task that has the file open
    size_t cursor; // location in the file the cursor is at
};

// Keep track of which processes have which files open
// also keeps track of where we are in the file (cursor)
struct FileOpenTable {

    // struct set to nullptr means the file is not open
    FileOpenTableEntry* entries[FS_BLOCK_COUNT];

};

void
fs_init(void)
{
    superblock = (Superblock) {
        .meta_list = {0}
    };
    open_table = (FileOpenTable) {
        .entries = {0}
    };
}

Fid
fs_create(char* filename)
{
    FileMeta* meta = arena_alloc(sizeof(FileMeta));
    *meta = (FileMeta) {
        .filename = filename, // TODO truncate filename passed
        .size = 0,
    };

    // linear search for spot to put new file
    for (unsigned int i = 0; i < FS_BLOCK_COUNT; ++i) {
        if (superblock.meta_list[i] == 0) {
            superblock.meta_list[i] = meta;
            return (Fid)i;
        }
    }
    // TODO handle max files allocated

}

Fid
fs_open(char* filename)
{
    // Check if file exists

    // Check that file isn't already open
    
    // Search for fid of filename

    // Put entry into open file table
    Fid fid = 0; // TODO temp
    FileOpenTableEntry* entry = arena_alloc(sizeof(FileOpenTableEntry));
    *entry = (FileOpenTableEntry) {
        .tid = tasktable_current_task(),
        .cursor = 0,
    };
    open_table.entries[fid] = entry;

    return fid;
}

void
fs_close(Fid fid)
{
    // TODO: maybe check if file is already open

    // Remove from open table
    arena_free(open_table.entries[fid]);
    open_table.entries[fid] = 0;
}
