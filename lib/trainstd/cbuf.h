#ifndef __TRAINSTD_CBUF_H__
#define __TRAINSTD_CBUF_H__

#include <stdint.h>
#include <stddef.h>
#include <traindef.h>

/* circular buffer implementation */

typedef struct CBuf CBuf;

CBuf* cbuf_new(size_t max_len);
void cbuf_delete(CBuf* cbuf);

u8 cbuf_front(CBuf* cbuf);
u8 cbuf_back(CBuf* cbuf);
int cbuf_push_front(CBuf* cbuf, u8 byte);
int cbuf_push_back(CBuf* cbuf, u8 byte);
u8 cbuf_pop_front(CBuf* cbuf);
u8 cbuf_pop_back(CBuf* cbuf);
u32 cbuf_len(CBuf* cbuf);

u8 cbuf_get(CBuf* cbuf, size_t index);
void cbuf_clear(CBuf* cbuf);
void cbuf_debug(CBuf* cbuf);

#endif // __TRAINSTD_CBUF_H__
