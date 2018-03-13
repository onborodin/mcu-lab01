
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

#define BAUD 19200
#include <util/setbaud.h>

#include <fifo.h>
#include <tools.h>
#include <shell.h>
#include <twim.h>

#include <ds3231.h>
#include <at24c.h>
#include <st7735.h>

#define regbit_set_up(reg, bit)    (reg) |= (1 << (bit))
#define regbit_set_down(reg, bit)  (reg) &= ~(1 << (bit))
#define regbit_is_set(reg, bit)    ((reg) & (1 << (bit)))
#define reg_set_value(reg, value)     ((reg) = (value))


static uint8_t inbuf[FIFO_BUFFER_SIZE];
static uint8_t outbuf[FIFO_BUFFER_SIZE];

fifo_t fifo_in, fifo_out;
fifo_t *in, *out;

int uart_putchar(char c, FILE * stream) {
    return fifo_putc(&fifo_out, c);
}

int uart_getchar(FILE * stream) {
    return (int)fifo_getc(&fifo_out);
}

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void io_hook(void) {
    in = &fifo_in;
    out = &fifo_out;

    fifo_init(in, inbuf, sizeof(inbuf));
    fifo_init(out, outbuf, sizeof(outbuf));

    stdout = &uart_str;
    stdin = &uart_str;
    stderr = &uart_str;
}

/* UART */
void uart_init(void) {
    /* UBRR - USART Baud Rate Register */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    /* UCSR  - USART Control and Status Register */
    /* U2X - Double Speed Operation */
    regbit_set_down(UCSR0A, U2X0);

    /* UCSZ - USART Character Size, 8 bit */
    regbit_set_down(UCSR0B, UCSZ02);
    regbit_set_up(UCSR0C, UCSZ01);
    regbit_set_up(UCSR0C, UCSZ00);

    /* USBS - USART Stop Bit Select */
    /* UPM - USART Parity Mode */
    /* One stop bit, no parity */
    regbit_set_down(UCSR0C, USBS0);
    regbit_set_down(UCSR0C, UPM00);
    regbit_set_down(UCSR0C, UPM01);

    /* Enable TX and RX */
    regbit_set_up(UCSR0B, TXEN0);
    regbit_set_up(UCSR0B, RXEN0);

    /* Enable Receive Interrupt */
    regbit_set_up(UCSR0B, RXCIE0);

    /* Disable Transmit Interrupt */
    regbit_set_down(UCSR0B, UDRIE0);
}

ISR(USART_RX_vect) {
    volatile uint8_t ichar = UDR0;

    if (ichar == '\r') {
        fifo_putc(in, '\n');
        fifo_putc(out, '\n');
    }

    fifo_putc(in, ichar);
    fifo_putc(out, ichar);
}

/* Watchdog timer */
void wdt_init(void) {
    wdt_enable(WDTO_30MS);
    WDTCSR = (1 << WDIE);
}

ISR(WDT_vect) {
    wdt_reset();
    volatile uint8_t c;
    while ((c = fifo_getc(out)) > 0) {
        while(!regbit_is_set(UCSR0A, UDRE0));
        UDR0 = c;
    }
    WDTCSR = (1 << WDIE);
}

/* Shell */
act_t shell_act[] = {
};

/* Timer0 */
void timer0_init(void) {
    //TCCR0B |= (1 << CS02) | (1 << CS00); /* CLK/1024 */
    TCCR0B |= (1 << CS02);                 /* CLK/256 */
    TIMSK0 |= (1 << TOIE0);
}

#define BUTTON_RELEASED  0
#define BUTTON_PRESSED   1

volatile uint8_t button_writed_state = BUTTON_RELEASED;
volatile uint16_t button_time = 0;

/* Timer 0 */
ISR(TIMER0_OVF_vect) {

#define button_just_released (!(PIND & (1 << PD4)))
#define button_just_pressed  (PIND & (1 << PD4))

    if (button_just_pressed) {
        if (button_writed_state == BUTTON_RELEASED) {
            button_writed_state = BUTTON_PRESSED;
            button_time++;
            printf("button pressed\r\n");
        } else if (button_writed_state == BUTTON_PRESSED) {
            button_time++;
        }
    }

    if (button_just_released) {
        if (button_writed_state == BUTTON_RELEASED) {
            /* nothing */
        } else if (button_writed_state == BUTTON_PRESSED) {
            button_writed_state = BUTTON_RELEASED;
            printf("button pressed time=%6d\r\n", button_time);
            button_time = 0;
        }
    }
}


#include <font8x14.h>


typedef struct font {
    uint8_t width;
    uint8_t height;
    uint8_t start;
    uint8_t length;
    const uint8_t *bitmap;
} font_t;

font_t basefont = {
    .width = 8,
    .height = 14,
    .start = 32,
    .length = 0x5F,
    .bitmap = basefont_bitmap
};


void lcd_draw_char(uint16_t xbase, uint16_t ybase, font_t *font, uint8_t c) {
    if (c < font->start || c > (font->start + font->length))
        c = 'x';
    ybase += font->width;
    xbase += font->height;
    c = c - font->start;
    for (uint8_t h = 0; h < font->height; h++) {
        for (uint8_t w = 0; w < font->width; w++) {

            if (pgm_read_byte(&(font->bitmap[(c) * font->height + h])) & (1 << w))
                lcd_draw_pixel((xbase - h), (ybase - w), 0xffff);
            else
                lcd_draw_pixel((xbase - h), (ybase - w), 0x0000);
        }
    }
}


typedef struct console {
    uint8_t width;
    uint8_t height;
    uint8_t line;
    uint8_t row;
    uint16_t xmax;
    uint16_t ymax;
    uint16_t xshift;
    uint16_t yshift;
    font_t *font;
    //uint8_t *data;
} console_t;


console_t console = {
    .width = 127/8,
    .height = 127/14 - 1,
    .line = 0,
    .row = 0,
    .xmax = 127,
    .ymax = 127,
    .xshift = 1,
    .yshift = -1,
    .font = &basefont
};

void console_print(console_t *console, uint8_t c) {
    if (c == '\r') {
        console->row = 0;
        return;
    }
    if (c == '\n') {
        console->row = 0;
        console->line++;
        return;
    }
    if ((console->row + 1) > console->width) {
        console->line++;
        console->row = 0;
    }
    lcd_draw_char(
            (console->xmax - (console->font->height * (console->line + 1))) + console->yshift,
            (console->font->width * console->row) + console->xshift,
            console->font, c);
    console->row++;

}


#define MAX_CMD_LEN 164

int main() {
    io_hook();
    uart_init();
    spi_init();
    lcd_init();
    i2c_init();
    wdt_init();
    //timer0_init();

    /* Set PIN as input */
    //regbit_set_down(DDRD, PD4);

    sei();

    _delay_ms(100);

    uint8_t str[MAX_CMD_LEN];
    uint8_t prompt[] = "READY>";
    fifo_puts(out, prompt);

    lcd_write_rect(0, 0, 127, 127, 0x0001);
    lcd_draw_line(0, 0, 127, 127, 0x5555);

    lcd_draw_line(0, 127, 127, 0, 0x5555);

    lcd_draw_vline(0, 0, 127, 0x5555);
    lcd_draw_hline(0, 0, 127, 0xFFFF);

    lcd_draw_char(64, 64, &basefont, 'A');


    uint8_t cs[] = "Hello, World!\r\nHello, World!";

    uint8_t n = 0;
    while (cs[n] != 0) {
        console_print(&console, cs[n]);
        n++;
    }


    while (1) {

        //for (uint8_t n = 0; n < 128; n += 2) {
        //    lcd_draw_rest(0, 0, n, n, lcd_rgb2color(0x05, 0xFF, 0x00));
        //}

        while (fifo_get_token(in, str, MAX_CMD_LEN, '\r') > 0) {
            int8_t ret_code = shell(str, shell_act, sizeof(shell_act) / sizeof(shell_act[0]));
            if (ret_code == SH_CMD_NOTFND)
                fifo_puts(out, (uint8_t *) "COMMAND NOT FOUND\r\n");
            fifo_puts(out, prompt);
        }
        _delay_ms(1);
    }
}
/* EOF */
