#ifndef __IO_H__
#define __IO_H__

#include <trainsys.h>
#include <trainstd.h>

#define IO_ADDRESS_MARKLIN "iom"
#define IO_ADDRESS_CONSOLE "ioc"

int Getc(Tid tid, int channel);
int Putc(Tid tid, int channel, unsigned char ch);

void marklinIO(void);
void consoleIO(void);

#endif // __IO_H__
