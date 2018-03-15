/* ADC */

#include <avr/io.h>


#define regbit_set_up(reg, bit)    (reg) |= (1 << (bit))
#define regbit_set_down(reg, bit)  (reg) &= ~(1 << (bit))
#define regbit_is_set(reg, bit)    ((reg) & (1 << (bit)))
#define reg_set_value(reg, value)     ((reg) = (value))

void adc_init() {
    regbit_set_up(ADMUX, REFS0);
    /* Enable ADC */
    reg_set_value(ADCSRA, ADEN);

    regbit_set_up(ADCSRA, ADPS2);
    regbit_set_up(ADCSRA, ADPS1);
    regbit_set_up(ADCSRA, ADPS0);       /* Set base freq prescale */
}

uint16_t adc_read(uint8_t ch) {
    if (ch > 7)
        return 0;

    ch &= (1 << MUX2) | (1 << MUX1) | (1 << MUX0); //0b00000111;
    ADMUX = (ADMUX & 0xf8) | ch;        /* Channel selection */
    ADCSRA |= (1 << ADSC);              /* Start conversion */

    while (ADCSRA & (1 << ADSC));

    return (ADC);
}

/* EOF */
