#ifndef __FS_H__
#define __FS_H__

#include <stddef.h>

#define MAX_FILENAME_LEN 32

#define FS_BLOCK_SIZE 1024
#define FS_BLOCK_COUNT 32

static unsigned char* const FS_BASE = (unsigned char*)0x01000000;

typedef size_t Fid;

typedef struct {
    char filename[MAX_FILENAME_LEN];
    size_t size;
} FileMeta;

void fs_init(void);

// Create a new file. Will error if filename already used
Fid fs_create(char* filename);

// Open an existing file
Fid fs_open(char* filename);

// Look up the Fid value for a given filename
Fid fs_lookup(char* filename);

// Get the metadata for a file
FileMeta fs_meta(Fid fid);

void fs_close(Fid fid);

#endif // __FS_H__
