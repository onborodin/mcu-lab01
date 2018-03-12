/* $Id$ */

#include <stdint.h>
#include <stdbool.h>

#include <twim.h>
#include <ds3231.h>

/*
    Example:

    ds_init();

    ds_set_month(12);
    ds_set_year(1999);
    ds_set_mday(31);

    ds_set_hour(23);
    ds_set_min(59);
    ds_set_sec(55);

    printf("%04d-%02d-%02d %02d:%02d:%02d\r\n", 
            ds_get_year(),
            ds_get_month(),
            ds_get_mday(),
            ds_get_hour(),
            ds_get_min(),
            ds_get_sec()
    );
 */


uint8_t ds_write(uint8_t reg, uint8_t data) {
    i2c_start_wait((DS3231_ADDR << 1) | I2C_WRITE); 
    i2c_write(reg);
    i2c_write(data);
    i2c_stop();
    return(1);
}

uint8_t ds_read(uint8_t reg) {
    volatile uint8_t data;
    i2c_start_wait((DS3231_ADDR << 1) | I2C_WRITE);
    i2c_write(reg);
    i2c_rep_start((DS3231_ADDR << 1) | I2C_READ);
    data = i2c_readNak();
    i2c_stop();
    return data;
}

volatile uint8_t ds_bcd2dec(uint8_t val) {
    return ((val & 0x0F) + (val >> 4) * 10);
}

uint8_t ds_dec2bcd(uint8_t val) {
    return (((val / 10) << 4) | ((val % 10) & 0x0F));
}

#define ds_clear_hi_bit(r) ((r) &= ~(1 << 7))

/* Get time functions */

/* Get sec */
uint8_t ds_get_sec(void) {
    uint8_t r = ds_read(DS_REGSEC);
    ds_clear_hi_bit(r);
    return ds_bcd2dec(r);
}
/* Get min */
uint8_t ds_get_min(void) {
    uint8_t r = ds_read(DS_REGMIN);
    ds_clear_hi_bit(r);
    return ds_bcd2dec(r);

}
/* Get hour */
uint8_t ds_get_hour(void) {
    uint8_t r = ds_read(DS_REGHOUR);
    ds_clear_hi_bit(r);
    /* Only 24h mode */
    r &= ~(1 << 6);
    return ds_bcd2dec(r);
}

/* Get date functions */

/* Get week day */
uint8_t ds_get_wday(void) {
    uint8_t r = ds_read(DS_REGWDAY);
    return r;
}

/* Get day */
uint8_t ds_get_mday(void) {
    uint8_t r = ds_read(DS_REGMDAY);
    return ds_bcd2dec(r);

}

/* Get month */
uint8_t ds_get_month(void) {
    uint8_t r = ds_read(DS_REGMONTH);
    /* Clear century bit */
    ds_clear_hi_bit(r);
    return ds_bcd2dec(r);

}

/* Get year */
uint16_t ds_get_year(void) {
    /* Get century bit */
    uint8_t cent = ds_read(DS_REGMONTH);
    cent &= (1 << 7);
    /* Get year low digits */
    uint8_t r = ds_read(DS_REGYEAR);
    r = ds_bcd2dec(r);
    /* Add century */
    return cent ? r + 2000 : r + 1900;
}

/* Time set functions */

/* Set sec */
void ds_set_sec(uint8_t num) {
    if (num > 59) 
        return;
    ds_write(DS_REGSEC, ds_dec2bcd(num));
}

/*  Set min */
void ds_set_min(uint8_t num) {
    if (num > 59) 
        return;
    ds_write(DS_REGMIN, ds_dec2bcd(num));
}

/* Set hour */
void ds_set_hour(uint8_t num) {
    if (num > 23) 
        return;
    ds_write(DS_REGHOUR, ds_dec2bcd(num));
}

/* Date set functions */

/* Set day */
void ds_set_mday(uint8_t num) {
    if (num > 31) 
        return;
    ds_write(DS_REGMDAY, ds_dec2bcd(num));
}

/* Set month  */
void ds_set_month(uint8_t num) {
    if (num > 12) 
        return;
    /* Read century bit */
    uint8_t cent = ds_read(DS_REGMONTH);
    cent &= (1 << 7);
    /* Pack month with century bit */
    ds_write(DS_REGMONTH, ds_dec2bcd(num) | cent);
}

/* Set year */
void ds_set_year(uint16_t num) {
    if (num < 1900 || num > 2099) 
        return;
    /* Get month byte */
    uint8_t month = ds_read(DS_REGMONTH);
    /* Set century bit to month byte */
    if (num < 2000) {
        month &= ~(1 << 7);
        num = num - 1900;
    } else {
        month |= (1 << 7);
        num = num - 2000;
    }
    /* Write month byte */
    ds_write(DS_REGMONTH, month);
    ds_write(DS_REGYEAR, ds_dec2bcd(num));
}

/* Init function, must use before any interrupt start */
void ds_init(void) {
    /* Start oscillator */
    uint8_t r;
    r  = ds_read(DS_REGST);
    r &= ~(1 << DS_BIT_OSF);
    ds_write(DS_REGST, r);

    /* Set 24h format */
    r = ds_read(DS_REGHOUR);
    r &= ~(1 << 6);
    ds_write(DS_REGHOUR, r);
}
/* EOF */
