#ifndef __DEV_DEV_H__
#define __DEV_DEV_H__

#if QEMU == 0
#define MMIO_BASE 0xFE000000
#endif

#if QEMU == 1
#define MMIO_BASE 0x3F000000
#endif


#endif // __DEV_DEV_H__
