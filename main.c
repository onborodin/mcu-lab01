
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
#include <console.h>

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
    return (int)fifo_getc(&fifo_out);
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
        fifo_putc(&fifo_in, '\n');
        fifo_putc(&fifo_out, '\n');
    }

    fifo_putc(&fifo_in, ichar);
    fifo_putc(&fifo_out, ichar);
}

/* Watchdog timer */
void wdt_init(void) {
    wdt_enable(WDTO_60MS);
    WDTCSR = (1 << WDIE);
}

ISR(WDT_vect) {
    wdt_reset();
    volatile uint8_t c;
    while ((c = fifo_getc(&fifo_out)) > 0) {
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
#if 0
    TCCR0B |= (1 << CS02) | (1 << CS00); /* CLK/1024 */
    TCCR0B |= (1 << CS01);                 /* CLK/256 */
    //TIMSK0 |= (1 << TOIE0);

    TCCR0A &= ~(1 << COM0A0);
    TCCR0A &= ~(1 << COM0A1);

    TCCR0A &= ~(1 << COM0B1); 
    TCCR0A &= ~(1 << COM0B0);
#endif
}

void button_init(void) {
    /* Set PIN as input */
    regbit_set_down(DDRD, PD3);
}

#define BUTTON_RELEASED  0
#define BUTTON_PRESSED   1

#define BUTTON_INFINITE_TIME 6400

typedef struct button {
    uint8_t writed_state;
    uint32_t release_time;
    uint32_t press_time;
    uint8_t prev_press;
} button_t;

static button_t button = { 
    .press_time = 0,
    .writed_state = BUTTON_RELEASED,
    .release_time = BUTTON_INFINITE_TIME
};

void ms(void) {

    uint16_t xmin = 0;
    uint16_t xmax = 127;
    uint16_t ymin = 0;
    uint16_t ymax = 127;

    for (uint16_t Px = xmin; Px < xmax; Px++) {
        for (uint16_t Py = ymin; Py < ymax; Py++) {
            lcd_draw_pixel(Px, Py, Px*Py);
        }
    }
}

void button_handler(button_t *button, uint8_t just_state) {

    if (just_state == BUTTON_PRESSED) {
        /* If button pressed */
        if (button->writed_state == BUTTON_RELEASED) {
            button->writed_state = BUTTON_PRESSED;
            button->release_time = 11;

        } else if (button->writed_state == BUTTON_PRESSED) {
            if (button->press_time < BUTTON_INFINITE_TIME) {
                button->press_time++;
            }
            button->writed_state = BUTTON_PRESSED;
        }
    } else if (just_state == BUTTON_RELEASED) {

        if (button->writed_state == BUTTON_RELEASED) {
            /* Measure release time */
            if (button->release_time < BUTTON_INFINITE_TIME) {
                button->release_time++;
            }
            button->writed_state = BUTTON_RELEASED;

        } else

        if (button->writed_state == BUTTON_PRESSED) {

            button->writed_state = BUTTON_RELEASED;
            if (button->press_time > 10 && button->press_time < 100 ) {
                //printf("short press\r\n");
                ms();
            } else if (button->press_time > 100 && button->press_time < 1000 ) {
                //printf("long press\r\n");
            }
            if (button->press_time > 0)
                printf(" -- pressed time=%6d, release time = %6d\r\n", button->press_time, button->release_time);

            button->press_time = 0;
        }
    }
}


#if 0
/* Timer 0 */
ISR(TIMER0_OVF_vect) {

#define button_just_released (!(PIND & (1 << PD3)))
#define button_just_pressed  (PIND & (1 << PD3))

    printf("timer int\r\n");

    if (button_just_pressed) {
            button_handler(&button, BUTTON_PRESSED);
    } else if (button_just_released) {
            button_handler(&button, BUTTON_RELEASED);
    }
}
#endif


#define MAX_CMD_LEN 164

int main() {
    io_hook();
    uart_init();
    spi_init();
    lcd_init();
    i2c_init();
    wdt_init();

    _delay_ms(100);

    //timer0_init();
    //button_init();

    sei();

    _delay_ms(100);
    lcd_clear();

    uint8_t str[MAX_CMD_LEN];
    uint8_t prompt[] = "READY>";
    console_puts(&console, prompt);
    printf(prompt);

    //ds_set_month(03);
    //ds_set_year(2018);
    //ds_set_mday(16);

    //ds_set_hour(14);
    //ds_set_min(19);
    //ds_set_sec(50);

    uint8_t date[12];
    uint8_t time[12];

    while (1) {

        sprintf(date, "%04d-%02d-%02d", ds_get_year(), ds_get_month(), ds_get_mday());
        sprintf(time, "%02d:%02d:%02d",  ds_get_hour(), ds_get_min(), ds_get_sec());
        console_xyputs(&console, 3, 3, date);
        console_xyputs(&console, 5, 4, time);

        while (fifo_get_token(&fifo_in, str, MAX_CMD_LEN, '\r') > 0) {
            int8_t ret_code = shell(str, shell_act, sizeof(shell_act) / sizeof(shell_act[0]));
            if (ret_code == SH_CMD_NOTFND)
                printf("COMMAND NOT FOUND\r\n");
            printf(prompt);
        }
        _delay_ms(50);
    }
}
/* EOF */
