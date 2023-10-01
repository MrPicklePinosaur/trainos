#ifndef __TRAINSTD_H__
#define __TRAINSTD_H__

#include <stddef.h>

#include "list.h"
#include "mem.h"
#include "cbuf.h"

// NOTE: this is a gcc extension
#define lambda(return_type, function_body) \
({ \
      return_type __fn__ function_body \
          __fn__; \
})

void println(char* format, ...);
void print(char* format, ...);
char getc(void);

#endif // __TRAINSTD_H__
