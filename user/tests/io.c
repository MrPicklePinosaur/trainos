#include <trainstd.h>
#include <traintasks.h>

void
putcTestTask(void)
{
    Tid clock_server = WhoIs(CLOCK_ADDRESS);
    Tid io_server = WhoIs(IO_ADDRESS_MARKLIN);
    Putc(io_server, 192);
    Putc(io_server, 26);
    Putc(io_server, 77);
    for (;;) {
        Putc(io_server, 133);
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        println("DATA GOTTEN: %d", Getc(io_server));
        Delay(clock_server, 1000);
    }
}
