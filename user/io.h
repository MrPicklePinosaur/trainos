#ifndef __IO_H__
#define __IO_H__

int Getc(Tid tid, int channel);
int Putc(Tid tid, int channel, unsigned char ch);

void marklinIO(void);

#endif // __IO_H__