#include <trainstd.h>
#include <trainsys.h>
#include <ctype.h>
#include "user/clock.h"
#include "user/io.h"
#include "user/nameserver.h"

#define CH_ENTER     0x0d
#define CH_BACKSPACE 0x08

// task for getting user input form the console
void
promptTask()
{
    Tid io_server = WhoIs(IO_ADDRESS_CONSOLE);

    CBuf* line = cbuf_new(32);

    for (;;) {
        // TODO export value of CONSOLE from uart.c
        int c = Getc(io_server, 1);

        if (isalnum(c) || isblank(c)) {
            cbuf_push_back(line, c);
        } else if (c == CH_ENTER) {
            // drain the buffer
            char completed_line[cbuf_len(line)];
            for (u8 i = 0; i < cbuf_len(line); ++i)
                completed_line[i] = cbuf_pop_front(line);

            //str8(completed_line);


        } else if (c == CH_BACKSPACE) {
            cbuf_pop_back(line);
        }
    }
}

// soley responsible for rendering the ui
void
uiTask()
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);

    // data structures

    for (;;) {

    }


    Exit();
}
