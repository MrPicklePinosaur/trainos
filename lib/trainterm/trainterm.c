#include "trainstd.h"
#include "./trainterm.h"
#include "render.h"

void
term_init(void)
{
    renderer_init();
    print("%s%s", ANSI_CLEAR, ANSI_HIDE);
}

void
term_clear(void)
{
    print("%s", ANSI_CLEAR);
}

void
term_render(void)
{

}
