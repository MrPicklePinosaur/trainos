#ifndef __MARKLIN_H__
#define __MARKLIN_H__

#include <stddef.h>
#include <stdint.h>

static const uint8_t SPEED_STOP     = 0x0;
static const uint8_t SPEED_REVERSE  = 0xF;

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

void marklin_init();
void marklin_train_ctl(uint32_t train, uint32_t speed);
void marklin_switch_ctl(uint32_t switch_id, SwitchMode mode);
void marklin_dump_s88(uint32_t count);
void marklin_go();
void marklin_stop();

#endif // __MARKLIN_H__
