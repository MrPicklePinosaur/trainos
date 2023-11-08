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
    char* s = alloc(sizeof(char)*3);
    s[0] = speed;
    s[1] = train;
    s[2] = 255;
    Puts(io_server, s);
}

void
marklin_switch_ctl(Tid io_server, u32 switch_id, SwitchMode mode)
{
    char* s;
    if (switch_id == 153 || switch_id == 154) {
        s = alloc(sizeof(char)*6);
        s[0] = mode;
        s[1] = switch_id;
        s[2] = mode == SWITCH_MODE_STRAIGHT ? SWITCH_MODE_CURVED : SWITCH_MODE_STRAIGHT;
        s[3] = switch_id == 153 ? 154 : 153;
        s[4] = 32;
        s[5] = 255;
    }
    else if (switch_id == 155 || switch_id == 156) {
        s = alloc(sizeof(char)*6);
        s[0] = mode;
        s[1] = switch_id;
        s[2] = mode == SWITCH_MODE_STRAIGHT ? SWITCH_MODE_CURVED : SWITCH_MODE_STRAIGHT;
        s[3] = switch_id == 155 ? 156 : 155;
        s[4] = 32;
        s[5] = 255;
    }
    else {
        s = alloc(sizeof(char)*4);
        s[0] = mode;
        s[1] = switch_id;
        s[2] = 32;
        s[3] = 255;
    }
    Puts(io_server, s);
}

void
marklin_dump_s88(Tid io_server, usize count)
{
    Putc(io_server, 128+count);
}

void
marklin_go(Tid io_server)
{
    char* s = alloc(sizeof(char)*3);
    s[0] = 96;
    s[1] = 96;
    s[2] = 255;
    Puts(io_server, s);
}

void
marklin_stop(Tid io_server)
{
    char* s = alloc(sizeof(char)*3);
    s[0] = 97;
    s[1] = 97;
    s[2] = 255;
    Puts(io_server, s);
}
