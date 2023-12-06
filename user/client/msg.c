#include <trainstd.h>
#include "msg.h"

#include "kern/dev/uart.h"

void
client_send_msg(ClientMsgType type, const char* msg, u32 size)
{
    const char MSG_START[2] = {0x69, 0x69};
    const u32 msg_len = (u32)size;
    const u32 msg_type = (u32)type; // assume enum is u32

    uart_put_size(CONSOLE, (const char*)&MSG_START, sizeof(MSG_START));
    uart_put_size(CONSOLE, (const char*)&msg_len, sizeof(msg_len));
    uart_put_size(CONSOLE, (const char*)&msg_type, sizeof(msg_type));
    uart_put_size(CONSOLE, (const char*)msg, msg_len);

}
