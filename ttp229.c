/* $Id$ */

#include <stdint.h>
#include <stdbool.h>

#include <twim.h>
#include <ttp229.h>

#define TTP229_ADDR  0x57

uint16_t ttp229_read(void) {
    volatile uint8_t dat0, dat1;
    i2c_start_wait((TTP229_ADDR << 1) | I2C_READ);
    dat0 = i2c_readAck();
    dat1 = i2c_readNak();
    i2c_stop();
    return (dat0 << 8) | dat1;
}
/* EOF */
