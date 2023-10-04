#!/bin/sh

# qemu-system-aarch64 -machine virt -cpu cortex-a72 -smp 6 -m 4G \
#     -kernel Image -append "root=/dev/vda2 rootfstype=ext4 rw panic=0 console=ttyAMA0" \
#     -drive format=raw,file=trainos.img,if=none,id=hd0,cache=writeback \
#     -device virtio-blk,drive=hd0,bootindex=0 \
#     -netdev user,id=mynet,hostfwd=tcp::2222-:22 \
#     -device virtio-net-pci,netdev=mynet \
#     -monitor telnet:127.0.0.1:5555,server,nowait

# qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 4G \
#     -kernel trainos.img \
#     -serial stdio

qemu-system-aarch64 -M raspi3b -kernel trainos.img -serial null -serial stdio -gdb tcp::1234
