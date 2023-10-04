#!/bin/sh

# qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 4G \
#     -kernel trainos.img \
#     -serial stdio

    # -nographic -serial mon:stdio \

qemu-system-aarch64 -M raspi3b \
    -kernel trainos.img \
    -serial null -serial stdio \
    -gdb tcp::1234

