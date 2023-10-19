#ifndef __GIC_H__
#define __GIC_H__

#include <traindef.h>

void gic_init(void);
u32 gic_read_iar(void);
void gic_write_eoir(u32 id);

#endif // __GIC_H__
