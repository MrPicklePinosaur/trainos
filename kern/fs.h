#ifndef __FS_H__
#define __FS_H__

#include <stddef.h>

#define MAX_FILENAME_LEN 32

#define FS_BLOCK_SIZE 1024
#define FS_BLOCK_COUNT 32

static unsigned char* const FS_BASE = (unsigned char*)0x01000000;

typedef size_t Fid;
typedef size_t Inode;

typedef struct {
    size_t size;
    Inode inode;
} FileMeta;

void fs_init(void);

Fid fs_open(char* filename);
void fs_close(Fid fid);

#endif // __FS_H__
