#include "user/io.h"
#include "marklin.h"

void
marklin_init(Tid io_server)
{
    Putc(io_server, 192);
}

void
marklin_train_ctl(Tid io_server, u32 train, u32 speed)
{
    Putc(io_server, speed);
    Putc(io_server, train);
}

void
marklin_switch_ctl(Tid io_server, u32 switch_id, SwitchMode mode)
{
    Putc(io_server, mode);
    Putc(io_server, switch_id);
    if (switch_id == 155) {
        mode == SWITCH_MODE_STRAIGHT ? Putc(io_server, SWITCH_MODE_CURVED) : Putc(io_server, SWITCH_MODE_STRAIGHT);
        Putc(io_server, 156);
    }
    else if (switch_id == 156) {
        mode == SWITCH_MODE_STRAIGHT ? Putc(io_server, SWITCH_MODE_CURVED) : Putc(io_server, SWITCH_MODE_STRAIGHT);
        Putc(io_server, 155);
    }
    else if (switch_id == 153) {
        mode == SWITCH_MODE_STRAIGHT ? Putc(io_server, SWITCH_MODE_CURVED) : Putc(io_server, SWITCH_MODE_STRAIGHT);
        Putc(io_server, 154);
    }
    else if (switch_id == 154) {
        mode == SWITCH_MODE_STRAIGHT ? Putc(io_server, SWITCH_MODE_CURVED) : Putc(io_server, SWITCH_MODE_STRAIGHT);
        Putc(io_server, 153);
    }
    Putc(io_server, 32);
}

void
marklin_dump_s88(Tid io_server, usize count)
{
    Putc(io_server, 128+count);
}

void
marklin_go(Tid io_server)
{
    Putc(io_server, 96);
    Putc(io_server, 96);
}

void
marklin_stop(Tid io_server)
{
    Putc(io_server, 97);
    Putc(io_server, 97);
}
