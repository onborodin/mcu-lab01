
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

#include <uart.h>
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

#define MAX_CMD_LEN 164

volatile uint16_t timer0 = 0;
volatile uint8_t ir_char = 0;
uint8_t in_str[MAX_CMD_LEN];

/* Timer0 */
void timer0_init(void) {
    TCCR0B |= (1 << CS02) | (1 << CS00); /* CLK/1024 */
    //TCCR0B |= (1 << CS01);               /* CLK/256 */

    regbit_set_up(TIMSK0, TOIE0);

    regbit_set_down(TCCR0A, COM0A0);
    regbit_set_down(TCCR0A, COM0A1);

    regbit_set_down(TCCR0A, COM0B1); 
    regbit_set_down(TCCR0A, COM0B0);
}


ISR(TIMER0_OVF_vect) {
}

#include <ir.h>

ir_t ir = {
    .id = 0x00FF
};


uint8_t * ir_get_str(ir_t *ir) {
    switch(ir_get_cmd(ir)) {
        case IR_BN_OK:
            return (uint8_t *)("OK");
            break;
        case IR_BN_LEFT:
            return (uint8_t *)("LEFT");
            break;
        case IR_BN_RIGHT:
            return (uint8_t *)("RIGHT");
            break;
        case IR_BN_UP:
            return (uint8_t *)("UP");
            break;
        case IR_BN_DOWN:
            return (uint8_t *)("DOWN");
            break;
        case IR_BN_1:
            return (uint8_t *)("1");
            break;
        case IR_BN_2:
            return (uint8_t *)("2");
            break;
        case IR_BN_3:
            return (uint8_t *)("3");
            break;
        case IR_BN_4:
            return (uint8_t *)("4");
            break;
        case IR_BN_5:
            return (uint8_t *)("5");
            break;
        case IR_BN_6:
            return (uint8_t *)("6");
            break;
        case IR_BN_7:
            return (uint8_t *)("7");
            break;
        case IR_BN_8:
            return (uint8_t *)("8");
            break;
        case IR_BN_9:
            return (uint8_t *)("9");
            break;
        case IR_BN_0:
            return (uint8_t *)("0");
            break;
        case IR_BN_AST:
            return (uint8_t *)("*");
            break;
        case IR_BN_NUM:
            return (uint8_t *)("#");
            break;
        default:
            return (uint8_t *)("NO");

    }
}



ISR(USART_RX_vect) {
    volatile uint8_t ichar = UDR0;
    fifo_putc(&fifo_in, ichar);
    ir_push_char(&ir, ichar);
}

int main() {
    io_hook();
    uart_init();
    spi_init();
    lcd_init();
    console_init();
    i2c_init();
    //wdt_init();
    timer0_init();

    _delay_ms(100);
    lcd_clear();
    _delay_ms(100);
    sei();


    uint8_t date[12];
    uint8_t time[12];
    uint8_t new_date[12];
    uint8_t new_time[12];

    sprintf(date, "%04d-%02d-%02d", ds_get_year(), ds_get_month(), ds_get_mday());
    sprintf(time, "%02d:%02d:%02d", ds_get_hour(), ds_get_min(), ds_get_sec());
    console_xyputs(&console, 3, 4, date);
    console_xyputs(&console, 5, 5, time);

    uint8_t prompt[] = "READY>";
    console_puts(&console, prompt);

    while (1) {

        sprintf(date, "%04d-%02d-%02d", ds_get_year(), ds_get_month(), ds_get_mday());
        sprintf(time, "%02d:%02d:%02d", ds_get_hour(), ds_get_min(), ds_get_sec());

        if (strcmp(date, new_date)) {
            strcpy(new_date, date);
            console_xyputs(&console, 3, 4, date);
        }
        if (strcmp(time, new_time)) {
            strcpy(new_time, time);
            console_xyputs(&console, 5, 5, time);

        }


        uint8_t ir_str[16];
        sprintf(ir_str, "IR 0x%02X %-06s", ir_get_cmd(&ir), ir_get_str(&ir));
        console_xyputs(&console, 7, 0, ir_str);
        _delay_ms(200);
    }
}
/* EOF */
