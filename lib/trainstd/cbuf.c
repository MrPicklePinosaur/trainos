#include <trainstd.h>

#include "cbuf.h"
#include "mem.h"

struct CBuf {
  u8* data;
  u32 front_ptr;
  u32 back_ptr;
  size_t len;
  size_t max_len;
};

CBuf*
cbuf_new(size_t max_len)
{
    u8* data_buf = alloc(sizeof(u8)*max_len);
    CBuf* cbuf = alloc(sizeof(CBuf));
    *cbuf = (CBuf) {
        .data = data_buf,
        .front_ptr = 0,
        .back_ptr = 0,
        .len = 0,
        .max_len = max_len,
    };
    return cbuf;
}

void
cbuf_delete(CBuf* cbuf)
{
    // TODO
    free(cbuf->data);
    free(cbuf);
}

u8
cbuf_front(CBuf* cbuf)
{
   return cbuf->data[cbuf->front_ptr];
}

u8
cbuf_back(CBuf* cbuf)
{
    // TODO handle if empty
    return cbuf->data[(cbuf->back_ptr-1) % cbuf->max_len];
}

// Returns 1 if buffer is full
int
cbuf_push_front(CBuf* cbuf, u8 byte)
{
    if (cbuf->len >= cbuf->max_len) return 1;

    cbuf->front_ptr = (cbuf->front_ptr-1) % cbuf->max_len;
    cbuf->data[cbuf->front_ptr] = byte;

    ++(cbuf->len);
    return 0;
}

// Returns 1 if buffer is full
int
cbuf_push_back(CBuf* cbuf, u8 byte)
{
    if (cbuf->len >= cbuf->max_len) return 1;

    cbuf->data[cbuf->back_ptr] = byte;
    cbuf->back_ptr = (cbuf->back_ptr+1) % cbuf->max_len;

    ++(cbuf->len);
    return 0;
}

u8
cbuf_pop_front(CBuf* cbuf)
{
    // TODO handle if buf is empty (this is bad default value)
    if (cbuf->len == 0) return 0;

    u8 data = cbuf->data[cbuf->front_ptr];

    cbuf->front_ptr = (cbuf->front_ptr+1) % cbuf->max_len;
    --(cbuf->len);

    return data;
}

u8
cbuf_pop_back(CBuf* cbuf)
{
    // TODO handle if buf is empty
    if (cbuf->len == 0) return 0;

    cbuf->back_ptr = (cbuf->back_ptr-1) % cbuf->max_len;
    u8 data = cbuf->data[cbuf->back_ptr];

    --(cbuf->len);

    return data;
}

u32
cbuf_len(CBuf* cbuf)
{
    return cbuf->len;
}

u8
cbuf_get(CBuf* cbuf, size_t index)
{
    if (index >= cbuf->len) return 0;
    return cbuf->data[(cbuf->front_ptr+index) % cbuf->max_len];
}

void
cbuf_clear(CBuf* cbuf)
{
    cbuf->len = 0;
    cbuf->front_ptr = 0;
    cbuf->back_ptr = 0;
}

void
cbuf_debug(CBuf* cbuf)
{
    for (unsigned int i = 0; i < cbuf_len(cbuf); ++i)
        print("%d ", cbuf_get(cbuf, i));
    print("\n");
}
