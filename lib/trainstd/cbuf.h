#ifndef __TRAINSTD_CBUF_H__
#define __TRAINSTD_CBUF_H__

#include <stdint.h>
#include <stddef.h>
#include <traindef.h>

/* circular buffer implementation */

typedef struct CBuf CBuf;

CBuf* cbuf_new(usize max_len);
void cbuf_delete(CBuf* cbuf);

void* cbuf_front(CBuf* cbuf);
void* cbuf_back(CBuf* cbuf);
int cbuf_push_front(CBuf* cbuf, void* data);
int cbuf_push_back(CBuf* cbuf, void* data);
void* cbuf_pop_front(CBuf* cbuf);
void* cbuf_pop_back(CBuf* cbuf);
usize cbuf_len(CBuf* cbuf);

void* cbuf_get(CBuf* cbuf, usize index);
void cbuf_clear(CBuf* cbuf);
void cbuf_debug(CBuf* cbuf);

#endif // __TRAINSTD_CBUF_H__
