#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/ui/parser.h"

void
testParser()
{
    println("Running test suite for parser -----------------");

    Arena arena = arena_new(256);

    TEST(parse_command(arena, str8("tr 24 15"))._type == PARSER_RESULT_TRAIN_SPEED);
    TEST(parse_command(arena, str8("rv 24"))._type == PARSER_RESULT_REVERSE);
    TEST(parse_command(arena, str8("sw 1 C"))._type == PARSER_RESULT_SWITCH);
    TEST(parse_command(arena, str8("sw 153 C"))._type == PARSER_RESULT_SWITCH);
    TEST(parse_command(arena, str8("light 25 on"))._type == PARSER_RESULT_LIGHTS);
    TEST(parse_command(arena, str8("go"))._type == PARSER_RESULT_GO);
    TEST(parse_command(arena, str8("stop"))._type == PARSER_RESULT_STOP);
    TEST(parse_command(arena, str8("q"))._type == PARSER_RESULT_QUIT);

    Exit();
}
