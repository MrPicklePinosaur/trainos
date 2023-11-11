
# Networking stack research


BCM2711 comes with a dedicated ethernet controller.

## Rpi boot process

When booting, the pi will first look for a SD card to boot from? If not found, it will switch over to netboot.

The idea is that obviously this bootcode has some sort of primitive networking
capabilities, and perhaps we are able to peek into the code that is responsible
for this.

## Resources

- [Writing a network stack](https://williamdurand.fr/2022/02/17/on-writing-a-network-stack-part-1/)
    - [src](https://github.com/librerpi/rpi-open-firmware/tree/master)
- [Rpi bootmodes](https://github.com/raspberrypi/documentation/blob/develop/documentation/asciidoc/computers/raspberry-pi/)
- [Rpi EEPROM image](https://github.com/raspberrypi/rpi-eeprom/)
- [PXE boot tutorial for RPi](https://linuxhit.com/raspberry-pi-pxe-boot-netbooting-a-pi-4-without-an-sd-card/)
