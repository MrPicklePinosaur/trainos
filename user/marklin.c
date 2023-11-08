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
    char s[] = {speed, train};
    Puts(io_server, s, countof(s));
}

void
marklin_switch_ctl(Tid io_server, u32 switch_id, SwitchMode mode)
{
    if (switch_id == 153 || switch_id == 154) {
        u32 second_switch = switch_id == 153 ? 154 : 153;
        SwitchMode second_mode = mode == SWITCH_MODE_STRAIGHT ? SWITCH_MODE_CURVED : SWITCH_MODE_STRAIGHT;
        unsigned char s[] = {mode, switch_id, second_mode, second_switch, 32};
        Puts(io_server, s, countof(s));

    }
    else if (switch_id == 155 || switch_id == 156) {
        u32 second_switch = switch_id == 155 ? 156 : 155;
        SwitchMode second_mode = mode == SWITCH_MODE_STRAIGHT ? SWITCH_MODE_CURVED : SWITCH_MODE_STRAIGHT;
        unsigned char s[] = {mode, switch_id, second_mode, second_switch, 32};
        Puts(io_server, s, countof(s));
    }
    else {
        unsigned char s[] = {mode, switch_id, 32};
        Puts(io_server, s, countof(s));
    }
}

void
marklin_dump_s88(Tid io_server, usize count)
{
    Putc(io_server, 128+count);
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
