#include "rpi.h"
#include <stdint.h>

#define MMIO_BASE 0xFE000000

static char* const TIMER_BASE = (char*)(MMIO_BASE + 0x00003000);

int kmain() {
  // initialize GPIO for both console and marklin uarts
  uart_init();

  uart_config_and_enable_marklin();

  uart_putc(MARKLIN, 192);

  // Wait 10ms before sending next command
  uint32_t wait_start = *(volatile uint32_t*)(TIMER_BASE + 0x04);
  while (*(volatile uint32_t*)(TIMER_BASE + 0x04) < wait_start + 100000);

  uint32_t prev_time = *(volatile uint32_t*)(TIMER_BASE + 0x04);
  uint32_t prev_sensor_state = 0;

  for (;;) {
    uart_putc(MARKLIN, 193);

    unsigned char sensor_state = uart_getc(MARKLIN);
    uart_getc(MARKLIN);

    uint32_t triggered = (~prev_sensor_state) & sensor_state;
    prev_sensor_state = sensor_state;

    // If A3 was triggered, record the time
    if ((triggered >> 5) & 0x1) {
      uint32_t time = *(volatile uint32_t*)(TIMER_BASE + 0x04);
      uart_printf(CONSOLE, "%d\r\n", time - prev_time);
      prev_time = time;
    }
  }
}
