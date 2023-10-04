<div align="center">

# TrainOS

microkernel developed for cs452

</div>

## Setting up for development

Copy `config.mk.example` and fill out the location of your toolchain directory:
```sh
cp config.mk.example config.mk
```

Build the project:
```sh
make
```

This should generate a `trainos.img` file. You can upload this image file to
the cs452 TFTP server.

On the raspberry pi, to boot the image:

```sh
dhcp
tftpboot 0x80000 129.97.167.60:images/<user>.img
go 0x80000
```

## Running in simulator

WARNING: currently qemu only supports raspberry pi 3, so there may be
unexpected between the simulator and the actual lab raspberry pis.

The kernel can be inside qemu and remotely debugged using gdb. Ensure that you
have the following installed:
- qemu-system-aarch64
- gdb-multiarch

First build the image with the `QEMU=true`. You can set the value in your
`config.mk` or pass it to make when building:
```sh
make clean; make QEMU=true
```

Start qemu with the trainos image:
```sh
./scripts/sim.sh
```
This will start a gdbserver on port 1234. Next start `gdb-multiarch` and run:
```sh
(gdb) target remote :1234
```

## Directory Structure

- `kern`: kernel code
- `user`: user programs
- `lib`: library implmentation
- `include`: header files for libraries

