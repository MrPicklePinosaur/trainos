#include "marklin.h"
#include "rpi.h"
#include "util.h"

void
Putc(int byte)
{
    uart_putc(MARKLIN, byte);
}

void
marklin_init()
{
    Putc(192);
    delay(100000);
}

void
marklin_train_ctl(uint32_t train, uint32_t speed)
{
    Putc(speed);
    Putc(train);
    delay(100000);
}

void
marklin_switch_ctl(uint32_t switch_id, SwitchMode mode)
{
    Putc(mode);
    Putc(switch_id);
    Putc(32);
    delay(100000);
}

void
marklin_dump_s88(uint32_t count)
{
    Putc(128+count);
    delay(100000);
}

void
marklin_pick_s88(size_t index)
{
    Putc(192+index);
    delay(100000);
}

void
marklin_go()
{
    Putc(96);
    Putc(96);
    delay(100000);
}

void
marklin_stop()
{
    Putc(97);
    Putc(97);
    delay(100000);
}
