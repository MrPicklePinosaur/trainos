#ifndef __UI_MARKLIN_H__
#define __UI_MARKLIN_H__

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
  SWITCH_MODE_STRAIGHT = 33,
  SWITCH_MODE_CURVED = 34,
} SwitchMode;

#endif // __UI_MARKLIN_H__
