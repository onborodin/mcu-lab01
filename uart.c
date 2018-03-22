
/* $Id$ */

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/eeprom.h>

#define BAUD 9600
#include <util/setbaud.h>

#include <fifo.h>
#include <tools.h>
#include <shell.h>
#include <twim.h>

#include <uart.h>

#define regbit_set_up(reg, bit)    (reg) |= (1 << (bit))
#define regbit_set_down(reg, bit)  (reg) &= ~(1 << (bit))
#define regbit_is_set(reg, bit)    ((reg) & (1 << (bit)))
#define reg_set_value(reg, value)     ((reg) = (value))

static uint8_t inbuf[FIFO_BUFFER_SIZE];
static uint8_t outbuf[FIFO_BUFFER_SIZE];

fifo_t fifo_in, fifo_out;

int uart_putchar(char c, FILE * stream) {
    return fifo_putc(&fifo_out, c);
}

int uart_getchar(FILE * stream) {
    return (int)fifo_getc(&fifo_in);
}

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void io_hook(void) {

    fifo_init(&fifo_in, inbuf, sizeof(inbuf));
    fifo_init(&fifo_out, outbuf, sizeof(outbuf));

    stdout = &uart_str;
    stdin = &uart_str;
    stderr = &uart_str;
}

/* UART */
void uart_init(void) {
    /* UBRR - USART Baud Rate Register */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    /* U2X - Double Speed Operation */
    regbit_set_down(UCSR0A, U2X0);
    regbit_set_down(UCSR0A, MPCM0);

    /* UCSZ - USART Character Size, 8 bit */
    regbit_set_down(UCSR0B, UCSZ02);
    regbit_set_up(UCSR0C, UCSZ01);
    regbit_set_up(UCSR0C, UCSZ00);

    /* Set async mode */
    regbit_set_down(UCSR0C, UMSEL01);
    regbit_set_down(UCSR0C, UMSEL00);

    /* One stop bit */
    regbit_set_down(UCSR0C, USBS0);
    /* No parity */
    regbit_set_down(UCSR0C, UPM00);
    regbit_set_down(UCSR0C, UPM01);

    /* Enable TX and RX */
    regbit_set_up(UCSR0B, TXEN0);
    regbit_set_up(UCSR0B, RXEN0);

    /* Enable Receive Interrupt */
    regbit_set_up(UCSR0B, RXCIE0);

    /* Disable Transmit Interrupt */
    regbit_set_down(UCSR0B, TXCIE0);
}


/* EOF */
