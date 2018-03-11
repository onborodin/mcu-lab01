/* $Id$ */

#ifndef AT24C_H_IUI
#define AT24C_H_IUI

#define AT24C_ADDR  0x57

void at24c_write(uint8_t reg, uint8_t data);
uint8_t at24c_read(uint8_t reg);

#endif
/* EOF */
