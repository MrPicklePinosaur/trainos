#ifndef __TRAINSTD_CONV_H__
#define __TRAINSTD_CONV_H__

/* conversion functions */
int a2d(char ch);
char a2i( char ch, char **src, int base, int *nump );
void ui2a( unsigned int num, unsigned int base, char *bf );
void i2a( int num, char *bf );


#endif // __TRAINSTD_CONV_H__
