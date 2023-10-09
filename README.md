<div align="center">

# TrainOS

microkernel developed for cs452

</div>

## Running the code

Copy `config.mk.example` and fill out the location of your toolchain directory:
```sh
cp config.mk.example config.mk
```

Build the project:
```sh
make
```

This should generate a `trainos.img` file. Upload this image file to [https://cs452.student.cs.uwaterloo.ca/](https://cs452.student.cs.uwaterloo.ca/). This should get it onto the Raspberry Pi.

Turn on the Raspberry Pi (or restart it if it's already on). Wait for the TrainOS kernel to boot up.

Below the TrainOS logo and the UW course gacha, there should be a prompt asking you to `SELECT TASK TO RUN`. Each task is assigned a number; type in the corresponding number to run that task. The RPS test is the task called `K2` while the performance test is the task called `K2Perf`.

You can find the CSV file for the K2 performance test at `docs/K2Perf.csv`.

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

