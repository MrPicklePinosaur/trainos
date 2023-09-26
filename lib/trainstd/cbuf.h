#ifndef __TRAINSTD_CBUF_H__
#define __TRAINSTD_CBUF_H__

#include <stdint.h>
#include <stddef.h>

/* circular buffer implementation */

typedef struct CBuf CBuf;

CBuf* cbuf_new(size_t max_len);
void cbuf_delete(CBuf* cbuf);

uint8_t cbuf_front(CBuf* cbuf);
uint8_t cbuf_back(CBuf* cbuf);
int cbuf_push(CBuf* cbuf, uint8_t byte);
uint8_t cbuf_pop(CBuf* cbuf);
uint32_t cbuf_len(CBuf* cbuf);

#endif // __TRAINSTD_CBUF_H__
