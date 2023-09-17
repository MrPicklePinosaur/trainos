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
