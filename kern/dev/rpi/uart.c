
#include "kern/dev/uart.h"

#if QEMU == 0

#include "kern/util.h"
#include "kern/dev/dev.h"
#include <stdarg.h>
#include <trainstd.h>

/*********** GPIO CONFIGURATION ********************************/

static char* const GPIO_BASE = (char*)(MMIO_BASE + 0x200000);
static const u32 GPFSEL_OFFSETS[6] = {0x00, 0x04, 0x08, 0x0c, 0x10, 0x14};
static const u32 GPIO_PUP_PDN_CNTRL_OFFSETS[4] = {0xe4, 0xe8, 0xec, 0xf0};

#define GPFSEL_REG(reg) (*(u32*)(GPIO_BASE + GPFSEL_OFFSETS[reg]))
#define GPIO_PUP_PDN_CNTRL_REG(reg) (*(u32*)(GPIO_BASE + GPIO_PUP_PDN_CNTRL_OFFSETS[reg]))

// function control settings for GPIO pins
static const u32 GPIO_INPUT  = 0x00;
static const u32 GPIO_OUTPUT = 0x01;
static const u32 GPIO_ALTFN0 = 0x04;
static const u32 GPIO_ALTFN1 = 0x05;
static const u32 GPIO_ALTFN2 = 0x06;
static const u32 GPIO_ALTFN3 = 0x07;
static const u32 GPIO_ALTFN4 = 0x03;
static const u32 GPIO_ALTFN5 = 0x02;

// pup/pdn resistor settings for GPIO pins
static const u32 GPIO_NONE = 0x00;
static const u32 GPIO_PUP  = 0x01;
static const u32 GPIO_PDP  = 0x02;

uint32_t query_gpio_func(uint32_t pin) {
  uint32_t reg   =  pin / 10;
  uint32_t shift = (pin % 10) * 3;
  uint32_t status = GPFSEL_REG(reg);   // read status
  status = (status >> shift) & 0x7;     // get the pin we want
  return(status);
}

uint32_t query_gpio_puppdn(uint32_t pin) {
  uint32_t reg   =  pin / 16;
  uint32_t shift = (pin % 16) * 2;
  uint32_t status = GPIO_PUP_PDN_CNTRL_REG(reg); // read status
  status = (status >> shift) & 0x3;
  return(status);
}

static void setup_gpio(u32 pin, u32 setting, u32 resistor) {
  u32 reg   =  pin / 10;
  u32 shift = (pin % 10) * 3;
  u32 status = GPFSEL_REG(reg);   // read status
  status &= ~(7u << shift);              // clear bits
  status |=  (setting << shift);         // set bits
  GPFSEL_REG(reg) = status;

  reg   =  pin / 16;
  shift = (pin % 16) * 2;
  status = GPIO_PUP_PDN_CNTRL_REG(reg); // read status
  status &= ~(3u << shift);              // clear bits
  status |=  (resistor << shift);        // set bits
  GPIO_PUP_PDN_CNTRL_REG(reg) = status; // write back
}

/*********** UART CONTROL ************************ ************/

static char* const UART0_BASE = (char*)(MMIO_BASE + 0x201000);
static char* const UART3_BASE = (char*)(MMIO_BASE + 0x201600);

// line_uarts[] maps the each serial line on the RPi hat to the UART that drives it
// currently:
//   * there is no line 0
//   * line 1 (console) is driven by RPi UART0
//   * line 2 (train control) is driven by RPi UART3
static char* const line_uarts[] = { NULL, UART0_BASE, UART3_BASE };

// UART register offsets
static const u32 UART_DR =   0x00;
static const u32 UART_FR =   0x18;
static const u32 UART_IBRD = 0x24;
static const u32 UART_FBRD = 0x28;
static const u32 UART_LCRH = 0x2c;
static const u32 UART_CR   = 0x30;
static const u32 UART_ILFS = 0x34;
static const u32 UART_IMSC = 0x38;
static const u32 UART_MIS  = 0x40;
static const u32 UART_ICR  = 0x44;

#define UART_REG(line, offset) (*(volatile u32*)(line_uarts[line] + offset))

// masks for specific fields in the UART registers
static const u32 UART_FR_RXFE = 0x10;
static const u32 UART_FR_TXFF = 0x20;
static const u32 UART_FR_RXFF = 0x40;
static const u32 UART_FR_TXFE = 0x80;

static const u32 UART_CR_UARTEN = 0x01;
static const u32 UART_CR_LBE = 0x80;
static const u32 UART_CR_TXE = 0x100;
static const u32 UART_CR_RXE = 0x200;
static const u32 UART_CR_RTS = 0x800;
static const u32 UART_CR_RTSEN = 0x4000;
static const u32 UART_CR_CTSEN = 0x8000;

static const u32 UART_LCRH_PEN = 0x2;
static const u32 UART_LCRH_EPS = 0x4;
static const u32 UART_LCRH_STP2 = 0x8;
static const u32 UART_LCRH_FEN = 0x10;
static const u32 UART_LCRH_WLEN_LOW = 0x20;
static const u32 UART_LCRH_WLEN_HIGH = 0x40;

static const u32 UART_IMSC_CTSMIM = 0x02; // transmit interrupt mask
static const u32 UART_IMSC_RXIM   = 0x10; // receive interrupt mask
static const u32 UART_IMSC_TXIM   = 0x20; // transmit interrupt mask

static const u32 UART_MIS_CTSMMIS = 0x02;
static const u32 UART_MIS_RXMIS   = 0x10;
static const u32 UART_MIS_TXMIS   = 0x20;

static const u32 UART_ICR_CTSMIC = 0x02;
static const u32 UART_ICR_RXIC   = 0x10;
static const u32 UART_ICR_TXIC   = 0x20;

CBuf* input_fifo;
CBuf* output_fifo;

void uart_config_and_enable(size_t line, u32 baudrate, u32 control, u32 interrupts);

// UART initialization, to be called before other UART functions
// Nothing to do for UART0, for which GPIO is configured during boot process
// For UART3 (line 2 on the RPi hat), we need to configure the GPIO to route
// the uart control and data signals to the GPIO pins expected by the hat
void uart_init() {

    input_fifo = cbuf_new(64);
    output_fifo = cbuf_new(64);

    setup_gpio(4, GPIO_ALTFN4, GPIO_NONE);
    setup_gpio(5, GPIO_ALTFN4, GPIO_PUP);
    setup_gpio(6, GPIO_ALTFN4, GPIO_PUP);
    setup_gpio(7, GPIO_ALTFN4, GPIO_NONE);
    setup_gpio(14, GPIO_ALTFN0, GPIO_NONE);
    setup_gpio(15, GPIO_ALTFN0, GPIO_PUP);

    // not strictly necessary, since line 1 is configured during boot
    // but we'll configure the line anyways, so we know what state it is in
    uart_config_and_enable(CONSOLE, 115200, UART_LCRH_WLEN_HIGH|UART_LCRH_WLEN_LOW, UART_IMSC_CTSMIM | UART_IMSC_RXIM);
    uart_config_and_enable(MARKLIN, 2400, UART_LCRH_WLEN_HIGH|UART_LCRH_WLEN_LOW|UART_LCRH_STP2, UART_IMSC_CTSMIM);
}

static const u32 UARTCLK = 48000000;

// Configure the line properties (e.g, parity, baud rate) of a UART
// and ensure that it is enabled
void uart_config_and_enable(size_t line, u32 baudrate, u32 control, u32 interrupts) {
    u32 cr_state;
    // to avoid floating point, this computes 64 times the required baud divisor
    u32 baud_divisor = (u32)((((u64)UARTCLK)*4)/baudrate);

    // line control registers should not be changed while the UART is enabled, so disable it
    cr_state = UART_REG(line, UART_CR);
    UART_REG(line, UART_CR) = cr_state & ~UART_CR_UARTEN;

    // set the baud rate
    UART_REG(line, UART_IBRD) = baud_divisor >> 6;
    UART_REG(line, UART_FBRD) = baud_divisor & 0x3f;

    // set the line control registers: 8 bit, no parity, 1 stop bit, FIFOs enabled
    UART_REG(line, UART_LCRH) = control;

    // enable interrupts
    UART_REG(line, UART_IMSC) = interrupts;

    // re-enable the UART
    // enable both transmit and receive regardless of previous state
    UART_REG(line, UART_CR) = cr_state | UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

unsigned char uart_getc(size_t line) {
  unsigned char ch;
  /* wait for data if necessary */
  while(UART_REG(line, UART_FR) & UART_FR_RXFE);
  ch = UART_REG(line, UART_DR);
  return(ch);
}

// Poll for input, return status is 1 if no data, 0 if data
int
uart_getc_poll(size_t line, unsigned char* data) {

  // return immediately if no data
  if (UART_REG(line, UART_FR) & UART_FR_RXFE) return (1);

  *data = UART_REG(line, UART_DR);
  return(0);
}

void uart_putc(size_t line, unsigned char c) {
  // make sure there is room to write more data
  while(UART_REG(line, UART_FR) & UART_FR_TXFF);
  UART_REG(line, UART_DR) = c;
}

// Return status 1 if byte was not written, 0 is successfully written
int uart_try_putc(size_t line, unsigned char c) {
  if (UART_REG(line, UART_FR) & UART_FR_TXFF) return (1);
  UART_REG(line, UART_DR) = c;
  return (0);
}

void uart_putl(size_t line, const char* buf, size_t blen) {
  u32 i;
  for(i=0; i < blen; i++) {
    uart_putc(line, *(buf+i));
  }
}

void uart_puts(size_t line, const char* buf) {
  while (*buf) {
    uart_putc(line, *buf);
    buf++;
  }
}

bool uart_busy(size_t line) {
  return UART_REG(line, UART_FR) & UART_FR_TXFF;
}

bool
uart_is_marklin_cts_interrupt(void) {
    if (UART_REG(MARKLIN, UART_MIS) & UART_MIS_CTSMMIS) {
        return true;
    }
    return false;
}

// clears all interrupts
void
uart_clear_interrupts(size_t line) {
    // read MIS to find out which interrupt was thrown
    u32 mis_reg = UART_REG(line, UART_MIS);
    if ((mis_reg & UART_MIS_RXMIS) == UART_MIS_RXMIS) {

        unsigned char data;
        if (uart_getc_poll(line, &data) == 1) {
            PANIC("no data on receive line");
        }
        cbuf_push_back(input_fifo, data); // TODO this may need mutual exclusion (since this interrupt may happen inside a cbuf operation)
        UART_REG(line, UART_ICR) = UART_ICR_RXIC;

    }
    if ((mis_reg & UART_MIS_TXMIS) == UART_MIS_TXMIS) {

        UART_REG(line, UART_ICR) = UART_ICR_TXIC;
    }
    if ((mis_reg & UART_MIS_CTSMMIS) == UART_MIS_CTSMMIS) {

        UART_REG(line, UART_ICR) = UART_ICR_CTSMIC;
    }
}

unsigned char
uart_getc_buffered(size_t line)
{
    if (cbuf_len(input_fifo) == 0) return 0;
    return (unsigned char)cbuf_pop_front(input_fifo);
}

#endif
