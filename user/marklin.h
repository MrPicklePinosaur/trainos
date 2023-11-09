#ifndef __UI_MARKLIN_H__
#define __UI_MARKLIN_H__

#include <trainsys.h>
#include <traindef.h>

static const u8 SPEED_STOP     = 0x0;
static const u8 SPEED_REVERSE  = 0xF;

typedef enum {
  SWITCH_GROUP_A = 0,
  SWITCH_GROUP_B,
  SWITCH_GROUP_C,
  SWITCH_GROUP_D,
  SWITCH_GROUP_E,
} SwitchGroup;

typedef enum {
  SWITCH_MODE_UNKNOWN = 0,
  SWITCH_MODE_STRAIGHT = 33,
  SWITCH_MODE_CURVED = 34,
} SwitchMode;

void marklin_init(Tid io_server);
void marklin_train_ctl(Tid io_server, u32 train, u32 speed);
void marklin_switch_ctl(Tid io_server, u32 switch_id, SwitchMode mode);
void marklin_dump_s88(Tid io_server, usize count);
void marklin_go(Tid io_server);
void marklin_stop(Tid io_server);

#endif // __UI_MARKLIN_H__
