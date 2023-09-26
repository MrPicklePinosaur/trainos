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
cbuf_push(CBuf* cbuf, uint8_t byte)
{
  if (cbuf->len >= cbuf->max_len) return 1;

  cbuf->data[cbuf->back_ptr] = byte;
  cbuf->back_ptr = (cbuf->back_ptr+1) % cbuf->max_len;

  ++(cbuf->len);
  return 0;
}

uint8_t
cbuf_pop(CBuf* cbuf)
{
  // TODO handle if buf is empty
  uint8_t data = cbuf->data[cbuf->front_ptr];

  cbuf->front_ptr = (cbuf->front_ptr+1) % cbuf->max_len;
  --(cbuf->len);

  return data;
}

uint32_t
cbuf_len(CBuf* cbuf)
{
  return cbuf->len;
}

/* tests

  CBuf out_stream = cbuf_new();
  uart_printf(CONSOLE, "%u\r\n", cbuf_len(&out_stream));
  cbuf_push(&out_stream, 0x1);
  uart_printf(CONSOLE, "%u\r\n", cbuf_len(&out_stream));
  cbuf_push(&out_stream, 0x2);
  cbuf_push(&out_stream, 0x3);
  uart_printf(CONSOLE, "%u\r\n", cbuf_len(&out_stream));
  uint8_t val = cbuf_pop(&out_stream);
  uart_printf(CONSOLE, "%u, val = %u\r\n", cbuf_len(&out_stream), val);

*/
