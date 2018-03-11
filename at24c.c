/* $Id$ */

#include <stdint.h>
#include <stdbool.h>

#include <twim.h>
#include <at24c.h>

/*
    Example:
    uint8_t str[] = "Hello, World!";
    uint8_t i = 0;
    while (str[i] != 0) {
        at24c_write(i, astr[i]);
        i++;
    }

    uint8_t i = 0;
    while (str[i] != 0) {
        printf("%c", at24c_read(i));
        i++;
    }
 */

void at24c_write(uint8_t reg, uint8_t data) {
    i2c_start_wait((AT24C_ADDR << 1) | I2C_WRITE); 
    i2c_write(reg);
    i2c_write(reg);
    i2c_write(data);
    i2c_stop();
}

uint8_t at24c_read(uint8_t reg) {
    volatile uint8_t data;
    i2c_start_wait((AT24C_ADDR << 1) | I2C_WRITE);
    i2c_write(reg);
    i2c_write(reg);
    i2c_rep_start((AT24C_ADDR << 1) | I2C_READ);
    data = i2c_readNak();
    i2c_stop();
    return data;
}

/* EOF */
