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
