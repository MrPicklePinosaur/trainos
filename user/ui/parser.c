#include <trainstd.h>
#include <ctype.h>
#include "parser.h"

str8 get_word(str8 command, u32* it);
u32 get_number(str8 command, u32* it);
i32 get_signed_number(str8 command, u32* it);
void eat_whitespace(str8 command, u32* it);

ParserResult
parse_command(Arena arena, str8 command)
{
    ParserResult error = (ParserResult) {
        ._type = PARSER_RESULT_ERROR,
    };

    // read until first whitespace character
    u32 it = 0;

    eat_whitespace(command, &it);

    str8 cmd_name = get_word(command, &it);

    if (str8_cmp(cmd_name, str8("tr"))) {

        eat_whitespace(command, &it);

        u32 train = get_number(command, &it);

        eat_whitespace(command, &it);

        u32 speed = get_number(command, &it);
        if (!(0 <= speed && speed <= 15)) return error;

        ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed TR command: train = %d, speed = %d", train, speed);

        return (ParserResult) {
            ._type = PARSER_RESULT_TRAIN_SPEED,
            ._data = {
                .train_speed = {
                    .train = train,
                    .speed = speed,
                }
            }
        };
    }
    else if (str8_cmp(cmd_name, str8("rv"))) {

        eat_whitespace(command, &it);

        int train = get_number(command, &it);

        ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed RV command: train = %d", train);

        return (ParserResult) {
            ._type = PARSER_RESULT_REVERSE,
            ._data = {
                .reverse = {
                    .train = train,
                }
            }
        };
    }
    else if (str8_cmp(cmd_name, str8("sw"))) {

        eat_whitespace(command, &it);

        int switch_id = get_number(command, &it);
        if (!((1 <= switch_id && switch_id <= 18) || (153 <= switch_id && switch_id <= 156))) return error;

        eat_whitespace(command, &it);

        str8 mode_str = get_word(command, &it);

        if (str8_cmp(mode_str, str8("S"))) {
            ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed SW command: switch_id = %d, mode = straight", switch_id);
            return (ParserResult) {
                ._type = PARSER_RESULT_SWITCH,
                ._data = {
                    .switch_control = {
                        .switch_id = switch_id,
                        .switch_mode = SWITCH_MODE_STRAIGHT
                    }
                },
            };
        }
        else if (str8_cmp(mode_str, str8("C"))) {
            ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed SW command: switch_id = %d, mode = curved", switch_id);
            return (ParserResult) {
                ._type = PARSER_RESULT_SWITCH,
                    ._data = {
                        .switch_control = {
                            .switch_id = switch_id,
                            .switch_mode = SWITCH_MODE_CURVED
                        }
                    },
                };
            }
        else {
            return error;
        }

    }
    else if (str8_cmp(cmd_name, str8("light"))) {
        eat_whitespace(command, &it);

        int train = get_number(command, &it);

        eat_whitespace(command, &it);

        str8 light_mode = get_word(command, &it);

        if (str8_cmp(light_mode, str8("on"))) {
            ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed LIGHT command: traind = %d, mode = on", train);
            return (ParserResult) {
                ._type = PARSER_RESULT_LIGHTS,
                ._data = {
                    .lights = {
                        .train = train,
                        .state = true,
                    }
                },
            };
        } else if (str8_cmp(light_mode, str8("off"))) {
            ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed LIGHT command: traind = %d, mode = off", train);
            return (ParserResult) {
                ._type = PARSER_RESULT_LIGHTS,
                ._data = {
                    .lights = {
                        .train = train,
                        .state = false,
                    }
                },
            };
        }
        return error;
    }
    else if (str8_cmp(cmd_name, str8("go"))) {
        ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed GO command");
        return (ParserResult) {
            ._type = PARSER_RESULT_GO,
        };
    }
    else if (str8_cmp(cmd_name, str8("stop"))) {
        ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed STOP command");
        return (ParserResult) {
            ._type = PARSER_RESULT_STOP,
        };
    }
    else if (str8_cmp(cmd_name, str8("q"))) {
        ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed QUIT command");
        return (ParserResult) {
            ._type = PARSER_RESULT_QUIT,
        };
    }
    else if (str8_cmp(cmd_name, str8("path"))) {

        eat_whitespace(command, &it);

        u32 train = get_number(command, &it);

        eat_whitespace(command, &it);

        str8 dest = get_word(command, &it);

        eat_whitespace(command, &it);

        u32 speed = get_number(command, &it);

        eat_whitespace(command, &it);

        i32 offset = get_signed_number(command, &it);

        ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed PATH command: train = %d, dest = %s, speed = %d, offset = %d", train, dest, speed, offset);

        // TODO dangling poitner here, so we are allocting some space
        str8 copied = str8_copy(&arena, dest);

        return (ParserResult) {
            ._type = PARSER_RESULT_PATH,
            ._data = {
                .path = {
                    .train = train,
                    .speed = speed,
                    .offset = offset,
                    .dest = str8_to_cstr(copied)
                }
            }
        };
    }
    else if (str8_cmp(cmd_name, str8("test"))) {

        eat_whitespace(command, &it);

        u32 num = get_number(command, &it);

        ULOG_DEBUG_M(LOG_MASK_PARSER, "Parsed TEST command: num = %d", num);

        return (ParserResult) {
            ._type = PARSER_RESULT_TEST,
            ._data = {
                .test = {
                    .num = num
                }
            }
        };

    }

    return error;
}

str8
get_word(str8 command, u32* it)
{
    u32 start = *it;
    while (1) {
        char c = str8_at(command, *it);
        if (c == 0) break;

        if (isspace(c)) break;

        ++(*it);
    }
    return str8_substr(command, start, *it);
}

u32
get_number(str8 command, u32* it)
{
  u32 number = 0;
  while (1) {
    char c = str8_at(command, *it);
    if (c == 0) break;

    if (!isdigit(c)) break;

    number = number*10 + (c-'0');
    
    ++(*it);
  }
  return number;
}

i32
get_signed_number(str8 command, u32* it)
{
  i32 number = 0;
  i32 sign = 1;
  usize char_ind = 0;
  while (1) {
    char c = str8_at(command, *it);

    if (char_ind == 0 && c == '+') {
        sign = 1;
    }
    else if (char_ind == 0 && c == '-') {
        sign = -1;
    }
    else if (isdigit(c)) {
        number = number*10 + (c-'0');
    } else {
        // invalid char, don't parse
        break;
    }

    ++(*it);
    ++char_ind;
  }
  return number*sign;
}

void
eat_whitespace(str8 command, u32* it)
{
  while (1) {
    char c = str8_at(command, *it);
    if (c == 0) break;

    if (!isspace(c)) break;
    
    ++(*it);
  }
}
