#ifndef __IO_H__
#define __IO_H__

#include <trainsys.h>
#include <trainstd.h>

#define IO_ADDRESS_MARKLIN "iom"
#define IO_ADDRESS_CONSOLE "ioc"

int Getc(Tid tid);
int Putc(Tid tid, unsigned char ch);
int Puts(Tid tid, unsigned char* s);  // String is delimited by 255

void marklinIO(void);
void consoleIO(void);

#endif // __IO_H__
