#include "rpi.h"
#include "kern.h"
#include "task.h"
#include "std.h"

// Serial line 1 on the RPi hat is used for the console

void mytask() {
    uint64_t timer_value = 0;
    uint64_t print_timer = 0;
    uint64_t yield_timer = 0;
    for (;;) {
        timer_value = timer_get();

        if (timer_value - print_timer > 1000000) {
            print_timer = timer_value;
            uart_printf(CONSOLE, "Hello from task");
        }

        if (timer_value - yield_timer > 3000000) {
            yield_timer = timer_value;
        }
    }
}

int kmain() {
    
    kern_init();

    Tid task0 = tasktable_create_task(0);
    Tid task1 = tasktable_create_task(0);

    // initialize both console and marklin uarts
    uart_init();

    // not strictly necessary, since line 1 is configured during boot
    // but we'll configure the line anyways, so we know what state it is in
    uart_config_and_enable(CONSOLE, 115200, 0x70);


    uart_printf(CONSOLE, "Welcome to TrainOS\r\n");

    char c = ' ';
    while (c != 'q') {
    c = uart_getc(CONSOLE);
        if (c == '\r') {
            uart_printf(CONSOLE, "asm_adder %d\r\n", asm_adder(21, 21));
            /* uart_printf(CONSOLE, "task0 %d\r\n", task0); */
            /* uart_printf(CONSOLE, "task1 %d\r\n", task1); */
        } else {
            uart_putc(CONSOLE, c);
        }
    }

    // U-Boot displays the return value from main - might be handy for debugging
    return 0;
}
