#ifndef __UI_PARSER_H__
#define __UI_PARSER_H__

#include <traindef.h>
#include <trainstd.h>
#include "user/marklin.h"

typedef struct {
  enum {
    PARSER_RESULT_TRAIN_SPEED,
    PARSER_RESULT_REVERSE,
    PARSER_RESULT_SWITCH,
    PARSER_RESULT_STOP,
    PARSER_RESULT_GO,
    PARSER_RESULT_LIGHTS,
    PARSER_RESULT_QUIT,
    PARSER_RESULT_PATH,
    PARSER_RESULT_ERROR,
  } _type;

  union {
    struct {
      u32 train;
      u32 speed;
    } train_speed;
    
    struct {
      u32 train;
    } reverse;

    struct {
      u32 switch_id;
      SwitchMode switch_mode;
    } switch_control;

    struct {
      u32 train;
      bool state;
    } lights;

    struct {
        u32 train;
        u32 speed;
        i32 offset;
        char* dest;
    } path;

  } _data;
} ParserResult;

ParserResult parse_command(Arena arena, str8 command);

#endif // __UI_PARSER_H__

