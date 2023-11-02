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

  uint32_t prev_time = 0;
  uint32_t start_time = 0;
  uint32_t prev_sensor_state = 0;

  uint32_t samples = 0;

  // Clear any previous sensor detections
  uart_putc(MARKLIN, 193);
  uart_getc(MARKLIN);
  uart_getc(MARKLIN);

  uart_putc(MARKLIN, 193);
  for (;;) {
    unsigned char sensor_state = uart_getc(MARKLIN);
    uart_getc(MARKLIN);

    // Mild optimization: Putc now so that we can do other things while waiting for reply
    uart_putc(MARKLIN, 193);

    uint32_t triggered = (~prev_sensor_state) & sensor_state;
    prev_sensor_state = sensor_state;

    // If A3 was triggered, record the time
    if ((triggered >> 5) & 0x1) {

      // Ignore first sensor trigger; we need the train to make a full loop first
      if (start_time == 0) {
        start_time = *(volatile uint32_t*)(TIMER_BASE + 0x04);
        prev_time = start_time;
      }

      else {
        uint32_t time = *(volatile uint32_t*)(TIMER_BASE + 0x04);
        samples++;
        uart_printf(CONSOLE, "This: %d   Avg: %d   Samples: %d\r\n", time - prev_time, (time - start_time) / samples, samples);
        prev_time = time;
      }
    }
  }
}
