#include <trainstd.h>

#include "cbuf.h"
#include "mem.h"

struct CBuf {
  uint8_t* data;
  uint32_t front_ptr;
  uint32_t back_ptr;
  size_t len;
  size_t max_len;
};

CBuf*
cbuf_new(size_t max_len)
{
    uint8_t* data_buf = alloc(sizeof(uint8_t)*max_len);
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

uint8_t
cbuf_front(CBuf* cbuf)
{
   return cbuf->data[cbuf->front_ptr];
}

uint8_t
cbuf_back(CBuf* cbuf)
{
    // TODO handle if empty
    return cbuf->data[(cbuf->back_ptr-1) % cbuf->max_len];
}

// Returns 1 if buffer is full
int
cbuf_push_front(CBuf* cbuf, uint8_t byte)
{
    if (cbuf->len >= cbuf->max_len) return 1;

    cbuf->front_ptr = (cbuf->front_ptr-1) % cbuf->max_len;
    cbuf->data[cbuf->front_ptr] = byte;

    ++(cbuf->len);
    return 0;
}

// Returns 1 if buffer is full
int
cbuf_push_back(CBuf* cbuf, uint8_t byte)
{
    if (cbuf->len >= cbuf->max_len) return 1;

    cbuf->data[cbuf->back_ptr] = byte;
    cbuf->back_ptr = (cbuf->back_ptr+1) % cbuf->max_len;

    ++(cbuf->len);
    return 0;
}

uint8_t
cbuf_pop_front(CBuf* cbuf)
{
    // TODO handle if buf is empty (this is bad default value)
    if (cbuf->len == 0) return 0;

    uint8_t data = cbuf->data[cbuf->front_ptr];

    cbuf->front_ptr = (cbuf->front_ptr+1) % cbuf->max_len;
    --(cbuf->len);

    return data;
}

uint8_t
cbuf_pop_back(CBuf* cbuf)
{
    // TODO handle if buf is empty
    if (cbuf->len == 0) return 0;

    cbuf->back_ptr = (cbuf->back_ptr-1) % cbuf->max_len;
    uint8_t data = cbuf->data[cbuf->back_ptr];

    --(cbuf->len);

    return data;
}

uint32_t
cbuf_len(CBuf* cbuf)
{
    return cbuf->len;
}

uint8_t
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
