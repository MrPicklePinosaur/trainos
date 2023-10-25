#include <trainstd.h>
#include <trainsys.h>
#include "tester.h"
#include "user/ui/parser.h"

void
testParser()
{
    println("Running test suite for parser -----------------");

    parse_command(str8("hello world"));

    Exit();
}
