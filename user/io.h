#ifndef __IO_H__
#define __IO_H__

#include <trainsys.h>
#include <trainstd.h>

#define IO_ADDRESS_MARKLIN "iom"
#define IO_ADDRESS_CONSOLE "ioc"

#define PUTS_BLOCK_SIZE 8

int Getc(Tid tid);
int Putc(Tid tid, unsigned char ch);
int Puts(Tid io_server, unsigned char* s, usize data_length);

void marklinIO(void);
void consoleIO(void);

#endif // __IO_H__
