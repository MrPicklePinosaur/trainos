#ifndef __GIC_H__
#define __GIC_H__

#include <stdint.h>

void gic_init(void);
uint32_t gic_read_iar(void);

#endif // __GIC_H__
