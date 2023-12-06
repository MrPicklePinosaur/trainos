#include <trainstd.h>
#include <traintasks.h>
#include "io.h"

#include "kern/dev/uart.h"

typedef struct {
    usize train;
    usize speed;
} IOMsg;

void
clientIoTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    for (;;) {
        Delay(clock_server, 100);

        IOMsg msg = (IOMsg) {
            .train = 2,
            .speed = 10
        };

        const char MSG_START[2] = {0x69, 0x69};
        const usize msg_len = sizeof(msg);

        uart_put_size(CONSOLE, (const char*)&MSG_START, sizeof(MSG_START));
        uart_put_size(CONSOLE, (const char*)&msg_len, sizeof(msg_len));
        uart_put_size(CONSOLE, (const char*)&msg, sizeof(msg));

    }
    Exit();
}
