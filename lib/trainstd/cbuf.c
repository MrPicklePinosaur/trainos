#include <trainstd.h>

#include "cbuf.h"
#include "mem.h"

struct CBuf {
  void** data;
  u32 front_ptr;
  u32 back_ptr;
  usize len;
  usize max_len;
};

CBuf*
cbuf_new(size_t max_len)
{
    void** data_buf = alloc(sizeof(void*)*max_len);
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

void*
cbuf_front(CBuf* cbuf)
{
   return cbuf->data[cbuf->front_ptr];
}

void*
cbuf_back(CBuf* cbuf)
{
    // TODO handle if empty
    return cbuf->data[(cbuf->back_ptr-1) % cbuf->max_len];
}

// Returns 1 if buffer is full
int
cbuf_push_front(CBuf* cbuf, void* data)
{
    if (cbuf->len >= cbuf->max_len) {
        return 1;
    }

    cbuf->front_ptr = (cbuf->front_ptr+cbuf->max_len-1) % cbuf->max_len;
    cbuf->data[cbuf->front_ptr] = data;

    ++(cbuf->len);
    return 0;
}

// Returns 1 if buffer is full
int
cbuf_push_back(CBuf* cbuf, void* data)
{
    if (cbuf->len >= cbuf->max_len) return 1;

    cbuf->data[cbuf->back_ptr] = data;
    cbuf->back_ptr = (cbuf->back_ptr+1) % cbuf->max_len;

    ++(cbuf->len);
    return 0;
}

void*
cbuf_pop_front(CBuf* cbuf)
{
    // TODO handle if buf is empty (this is bad default value)
    if (cbuf->len == 0) return 0;

    void* data = cbuf->data[cbuf->front_ptr];

    cbuf->front_ptr = (cbuf->front_ptr+1) % cbuf->max_len;
    --(cbuf->len);

    return data;
}

void*
cbuf_pop_back(CBuf* cbuf)
{
    // TODO handle if buf is empty
    if (cbuf->len == 0) return 0;

    cbuf->back_ptr = (cbuf->back_ptr-1) % cbuf->max_len;
    void* data = cbuf->data[cbuf->back_ptr];

    --(cbuf->len);

    return data;
}

usize
cbuf_len(CBuf* cbuf)
{
    return cbuf->len;
}

void*
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
