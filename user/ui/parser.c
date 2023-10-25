#include <trainstd.h>
#include <ctype.h>
#include "parser.h"

str8 get_word(str8 command, u32* it);
u32 get_number(str8 command, u32* it);
void eat_whitespace(str8 command, u32* it);

ParserResult
parse_command(str8 command)
{
    // read until first whitespace character
    u32 it = 0;

    str8 cmd_name = get_word(command, &it);
    ULOG_DEBUG("%s\n ", str8_to_cstr(cmd_name));

    return (ParserResult) {
        ._type = PARSER_RESULT_ERROR,
    };
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
