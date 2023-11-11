#include <traintasks.h>
#include "marklin.h"

void
marklin_init(Tid io_server)
{
    Putc(io_server, 192);
}

void
marklin_train_ctl(Tid io_server, u32 train, u32 speed)
{
    char s[] = {speed, train};
    Puts(io_server, s, countof(s));
}

void
marklin_switch_ctl(Tid io_server, u32 switch_id, SwitchMode mode)
{
    unsigned char s[] = {mode, switch_id, 32};
    Puts(io_server, s, countof(s));
}

void
marklin_dump_s88(Tid io_server, usize count)
{
    Putc(io_server, 128+count);
}

void
marklin_get_s88(Tid io_server, usize index)
{
    Putc(io_server, 192+index);
}

void
marklin_go(Tid io_server)
{
    char s[] = {96, 96};
    Puts(io_server, s, countof(s));
}

void
marklin_stop(Tid io_server)
{
    char s[] = {97, 97};
    Puts(io_server, s, countof(s));
}
