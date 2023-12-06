#include <trainstd.h>
#include <traintasks.h>
#include "io.h"

#include "kern/dev/uart.h"

typedef enum {
    CLIENT_MSG_SENSOR = 1,
} ClientMsgType;

typedef struct {
    usize train;
    usize speed;
} ClientMsg;

void
client_send_msg()
{

}

void
clientIoTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    usize i = 0;
    for (;;++i) {
        Delay(clock_server, 100);

        ClientMsg msg = (ClientMsg) {
            .train = 2,
            .speed = i
        };

        const char MSG_START[2] = {0x69, 0x69};
        const u32 msg_len = sizeof(msg);
        const u32 msg_type = CLIENT_MSG_SENSOR; // assume enum is u32

        uart_put_size(CONSOLE, (const char*)&MSG_START, sizeof(MSG_START));
        uart_put_size(CONSOLE, (const char*)&msg_len, sizeof(msg_len));
        uart_put_size(CONSOLE, (const char*)&msg_type, sizeof(msg_type));
        uart_put_size(CONSOLE, (const char*)&msg, sizeof(msg));

    }
    Exit();
}
