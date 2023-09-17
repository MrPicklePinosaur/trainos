#include "rpi.h"

// Serial line 1 on the RPi hat is used for the console
static const size_t CONSOLE = 1;

int kmain() {
  char hello[] = "Hello world, this is iotest (" __TIME__ ")\r\nPress 'q' to reboot\r\n";

  // initialize both console and marklin uarts
  uart_init();

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in
  uart_config_and_enable(CONSOLE, 115200);

  uart_puts(CONSOLE, hello);

  unsigned int counter=1;
  uart_printf(CONSOLE, "PI[%u]> ", counter++);
  char c = ' ';
  while (c != 'q') {
    c = uart_getc(CONSOLE);
    if (c == '\r') {
      uart_printf(CONSOLE, "\r\nPI[%u]> ", counter++);
    } else {
      uart_putc(CONSOLE, c);
    }
  }
  uart_puts(CONSOLE, "\r\n");

  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}
