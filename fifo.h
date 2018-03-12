/* $Id$ */

#ifndef UART_H_IUI
#define UART_H_IUI

#ifndef FIFO_BUFFER_SIZE
#define FIFO_BUFFER_SIZE 128
#endif

typedef struct fifo {
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t *buffer;
    uint8_t buffer_len;
} fifo_t;

void fifo_init(fifo_t * b, uint8_t * buffer, uint8_t buffer_len);
uint8_t fifo_count(const fifo_t * b);
bool fifo_full(const fifo_t * b);
bool fifo_empty(const fifo_t * b);
uint8_t fifo_peek(const fifo_t * b);
uint8_t fifo_getc(fifo_t * b);
bool fifo_putc(fifo_t * b, uint8_t data);
uint8_t fifo_puts(fifo_t * b, uint8_t * str);
bool fifo_scanc(fifo_t * b, uint8_t c);
uint8_t fifo_get_token(fifo_t * b, uint8_t * str, uint8_t len, uint8_t);
bool fifo_back(fifo_t * b);


#endif
/* EOF */
