#include <trainstd.h>
#include <traintasks.h>
#include "render.h"

void
FlushWin(Window* win)
{

}

void
traintermTask()
{
    // round robin with a bucket for each task

    RegisterAs(TRAINTERM_ADDRESS);
}
