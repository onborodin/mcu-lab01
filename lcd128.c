
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
//#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
//#include <util/atomic.h>
#include <util/delay.h>

#include <lcd128.h>

#define SPI_CLOCK_DIV4    0x00
#define SPI_CLOCK_DIV16   0x01
#define SPI_CLOCK_DIV64   0x02
#define SPI_CLOCK_DIV128  0x03
#define SPI_CLOCK_DIV2    0x04
#define SPI_CLOCK_DIV8    0x05
#define SPI_CLOCK_DIV32   0x06

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK    0x0C   /* CPOL = bit 3, CPHA = bit 2 on SPCR */
#define SPI_CLOCK_MASK   0x03   /* SPR1 = bit 1, SPR0 = bit 0 on SPCR */
#define SPI_2XCLOCK_MASK 0x01   /* SPI2X = bit 0 on SPSR */

#define PIN_SCK   PB5
#define PIN_MISO  PB4
#define PIN_MOSI  PB3
#define PIN_SS    PB2
#define PIN_RESET PB1
#define PIN_A0    PB0

#define mmio_set_up_bite(reg, bite)    (reg) |= (1 << (bite))
#define mmio_set_down_bite(reg, bite)  (reg) &= ~(1 << (bite))

void spi_enable(void) {
    mmio_set_up_bite(SPCR, SPE);
}

void spi_disable(void) {
    mmio_set_down_bite(SPCR, SPE);
}

void spi_init(void) {
    /* Set PIN_MISO as input */
    mmio_set_down_bite(DDRB, PIN_MISO); 

    /* Set PIN_SS as output */
    mmio_set_up_bite(DDRB, PIN_SS);     
    mmio_set_up_bite(DDRB, PIN_MOSI);
    mmio_set_up_bite(DDRB, PIN_SCK);
    mmio_set_up_bite(DDRB, PIN_RESET);
    mmio_set_up_bite(DDRB, PIN_A0);

    /* Down SS */
    mmio_set_up_bite(PORTB, PIN_SS); 

    /* Set master mode */
    mmio_set_up_bite(SPCR, MSTR);  
    /* Set speed */
    mmio_set_down_bite(SPCR, SPR0);
    mmio_set_down_bite(SPCR, SPR1);  
    /* Set duble speed */
    mmio_set_up_bite(SPCR, SPI2X);   

    /* Set order */
    mmio_set_down_bite(SPCR, DORD);  

    /* Set mode */
    mmio_set_down_bite(SPCR, CPOL);  
    mmio_set_down_bite(SPCR, CPHA);

    spi_enable();
}

inline void spi_write_byte(uint8_t c) {
    SPDR = c;
    while (!(SPSR & (1 << SPIF)));
}

void lcd_write_command(uint8_t c) {
    mmio_set_down_bite(PORTB, PIN_A0);
    mmio_set_down_bite(PORTB, PIN_SS);
    spi_write_byte(c);
    mmio_set_up_bite(PORTB, PIN_A0);
}

inline void lcd_write_byte(uint8_t c) {
    mmio_set_up_bite(PORTB, PIN_A0);
    mmio_set_down_bite(PORTB, PIN_SS);
    spi_write_byte(c);
    mmio_set_up_bite(PORTB, PIN_A0);
}

void lcd_write_word(uint16_t c) {
    mmio_set_up_bite(PORTB, PIN_A0);
    mmio_set_down_bite(PORTB, PIN_SS);
    spi_write_byte(c >> 8);
    spi_write_byte(c & 0xff);
    mmio_set_up_bite(PORTB, PIN_A0);
}

void lcd_write_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {

    lcd_addr_window(x, y, (x + w) + 1, (y + h) + 1);

    mmio_set_up_bite(PORTB, PIN_A0);
    mmio_set_down_bite(PORTB, PIN_SS);

    for (uint8_t i = 0; i < h; i++) {
        for (uint8_t j = 0; j < w; j++) {
            spi_write_byte(color >> 8);
            spi_write_byte(color & 0xff);
        }
    }
    mmio_set_up_bite(PORTB, PIN_A0);
}


void lcd_reset(void) {
    mmio_set_up_bite(PORTB, PIN_RESET);
    _delay_ms(100);
    mmio_set_down_bite(PORTB, PIN_RESET);
    _delay_ms(100);
    mmio_set_up_bite(PORTB, PIN_RESET);
    _delay_ms(100);
}

void lcd_init(void) {

    //lcd_reset();
    mmio_set_up_bite(PORTB, PIN_RESET);

    /* 1: Software reset, 0 args, w/delay */
    lcd_write_command(ST7735_SWRESET);
    /* 150 ms delay */
    _delay_ms(150);

    /* 2: Out of sleep mode, 0 args, w/delay */
    lcd_write_command(ST7735_SLPOUT);
    /* 500 ms delay */
    _delay_ms(500);

    /* 3: Frame rate ctrl - normal mode, 3 args: */
    lcd_write_command(ST7735_FRMCTR1);
    /* Rate = fosc/(1x2+40) * (LINE+2C+2D) */
    lcd_write_byte(0x01);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x2D);

    /* 4: Frame rate control - idle mode, 3 args: */
    lcd_write_command(ST7735_FRMCTR2);
    /* Rate = fosc/(1x2+40) * (LINE+2C+2D) */
    lcd_write_byte(0x01);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x2D);

    /* 5: Frame rate ctrl - partial mode, 6 args: */
    lcd_write_command(ST7735_FRMCTR3);
    /* Dot inversion mode */
    lcd_write_byte(0x01);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x2D);
    /* Line inversion mode */
    lcd_write_byte(0x01);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x2D);

    /* 6: Display inversion ctrl, 1 arg, no delay: */
    lcd_write_command(ST7735_INVCTR);
    /* No inversion */
    lcd_write_byte(0x07);

    /* 7: Power control, 3 args, no delay: */
    lcd_write_command(ST7735_PWCTR1);
    lcd_write_byte(0xA2);
    /* -4.6V */
    lcd_write_byte(0x02);
    /* AUTO mode */
    lcd_write_byte(0x84);

    /* 8: Power control, 1 arg, no delay: */
    lcd_write_command(ST7735_PWCTR2);
    /* VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD */
    lcd_write_byte(0xC5);

    /* 9: Power control, 2 args, no delay: */
    lcd_write_command(ST7735_PWCTR3);
    /* Opamp current small */
    lcd_write_byte(0x0A);
    /* Boost frequency */
    lcd_write_byte(0x00);

    /* 10: Power control, 2 args, no delay: */
    lcd_write_command(ST7735_PWCTR4);
    /* BCLK/2, Opamp current small & Medium low */
    lcd_write_byte(0x8A);
    lcd_write_byte(0x2A);

    /*  11: Power control, 2 args, no delay: */
    lcd_write_command(ST7735_PWCTR5);
    lcd_write_byte(0x8A);
    lcd_write_byte(0xEE);

    /* 12: Power control, 1 arg, no delay: */
    lcd_write_command(ST7735_VMCTR1);
    lcd_write_byte(0x0E);

    /* 13: Don't invert display, no args, no delay */
    lcd_write_command(ST7735_INVOFF);

    /* 14: Memory access control (directions), 1 arg: */
    lcd_write_command(ST7735_MADCTL);
    /* row addr/col addr, bottom to top refresh, RGB order */
    lcd_write_byte(0xC0);

    /* 15: Set color mode, 1 arg + delay: */
    lcd_write_command(ST7735_COLMOD);
    /* 16-bit color 5-6-5 color format */
    lcd_write_byte(0x05);
    /* 10 ms delay */
    _delay_ms(10);

    /* 1: Column addr set, 4 args, no delay */
    lcd_write_command(ST7735_CASET);
    /* XSTART = 0 */
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    /* XEND = 127 */
    lcd_write_byte(0x00);
    lcd_write_byte(0x7F);
    /* 2: Row addr set, 4 args, no delay: */
    lcd_write_command(ST7735_RASET);
    /* XSTART = 0 */
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    /* XEND = 127 */
    lcd_write_byte(0x00);
    lcd_write_byte(0x7F);

    /* 1: Magical unicorn dust, 16 args, no delay: */
    lcd_write_command(ST7735_GMCTRP1);
    lcd_write_byte(0x02);
    lcd_write_byte(0x1c);
    lcd_write_byte(0x07);
    lcd_write_byte(0x12);
    lcd_write_byte(0x37);
    lcd_write_byte(0x32);
    lcd_write_byte(0x29);
    lcd_write_byte(0x2d);
    lcd_write_byte(0x29);
    lcd_write_byte(0x25);
    lcd_write_byte(0x2B);
    lcd_write_byte(0x39);
    lcd_write_byte(0x00);
    lcd_write_byte(0x01);
    lcd_write_byte(0x03);
    lcd_write_byte(0x10);

    /* 2: Sparkles and rainbows, 16 args, no delay: */
    lcd_write_command(ST7735_GMCTRN1);
    lcd_write_byte(0x03);
    lcd_write_byte(0x1d);
    lcd_write_byte(0x07);
    lcd_write_byte(0x06);
    lcd_write_byte(0x2E);
    lcd_write_byte(0x2C);
    lcd_write_byte(0x29);
    lcd_write_byte(0x2D);
    lcd_write_byte(0x2E);
    lcd_write_byte(0x2E);
    lcd_write_byte(0x37);
    lcd_write_byte(0x3F);
    lcd_write_byte(0x00);
    lcd_write_byte(0x00);
    lcd_write_byte(0x02);
    lcd_write_byte(0x10);

    /*  3: Normal display on, no args, w/delay */
    lcd_write_command(ST7735_NORON);
    _delay_ms(10);

    /*  4: Main screen turn on, no args w/delay */
    lcd_write_command(ST7735_DISPON);
    _delay_ms(100);
}

#define LCD_DELTA_X     1
#define LCD_DELTA_Y     3


void lcd_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

    lcd_write_command(ST7735_CASET);
    lcd_write_byte(0x00);
    lcd_write_byte(x0 + LCD_DELTA_X);
    lcd_write_byte(0x00);
    lcd_write_byte(x1 + LCD_DELTA_X);

    lcd_write_command(ST7735_RASET);
    lcd_write_byte(0x00);
    lcd_write_byte(y0 + LCD_DELTA_Y);
    lcd_write_byte(0x00);
    lcd_write_byte(y1 + LCD_DELTA_Y);

    lcd_write_command(ST7735_RAMWR);
}

void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color) {
    lcd_addr_window(x, y, x + 1, y + 1);
    lcd_write_word(color);
}


/* EOF */
