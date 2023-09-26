#include "fs.h"
#include "task.h"

typedef struct Superblock Superblock;
typedef struct FileOpenTable FileOpenTable;

static Superblock superblock;
static FileOpenTable open_table;

struct Superblock {
    // map file names to a location (inode)
    FileMeta* meta[FS_BLOCK_COUNT];
};


// Keep track of which processes have which files open
// also keeps track of where we are in the file (cursor)
struct FileOpenTable {

    // struct set to nullptr means the file is not open
    struct {
        Tid tid; // the task that has the file open
        size_t cursor; // location in the file the cursor is at
    }* entries[FS_BLOCK_COUNT];

};

void
fs_init(void)
{
    superblock = (Superblock) {
        .meta = {0}
    };
    open_table = (FileOpenTable) {
        .entries = {0}
    };
}
