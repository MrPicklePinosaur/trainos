
#include <trainstd.h>
#include "kern/dev/uart.h"

// printf-style printing, with limited format support
void uart_format_print (size_t line, char *fmt, va_list va ) {
	char bf[12];
	char ch;

  while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			uart_putc(line, ch );
		else {
			ch = *(fmt++);
			switch( ch ) {
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				uart_puts( line, bf );
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				uart_puts( line, bf );
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				uart_puts( line, bf );
				break;
			case 's':
				uart_puts( line, va_arg( va, char* ) );
				break;
            case 'c':
                uart_putc( line, va_arg( va, int ) );
                break;
			case '%':
				uart_putc( line, ch );
				break;
      case '\0':
        return;
			}
		}
	}
}

void uart_printf( size_t line, char *fmt, ... ) {
	va_list va;

	va_start(va,fmt);
	uart_format_print( line, fmt, va );
	va_end(va);
}

void uart_put_size(size_t line, const char* buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        uart_putc(line, buf[i]);
    }
}
