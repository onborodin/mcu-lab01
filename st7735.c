/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <st7735.h>

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

#define MADCTL_MY       0x80
#define MADCTL_MX       0x40
#define MADCTL_MV       0x20
#define MADCTL_ML       0x10
#define MADCTL_RGB      0x00
#define MADCTL_BGR      0x08
#define MADCTL_MH       0x04


#define regbit_set_up(reg, bite)    (reg) |= (1 << (bite))
#define regbit_set_down(reg, bite)  (reg) &= ~(1 << (bite))

#ifndef ST7735_TFTWIDTH
#define ST7735_TFTWIDTH  127
#endif
#ifdef ST7735_TFTHEIGHT
#define ST7735_TFTHEIGHT 127
#endif

typedef struct lcd_screen {
    uint16_t width;
    uint16_t height;
} lcd_screen_t;

lcd_screen_t lcd_screen = { 
    .width = ST7735_TFTWIDTH,
    .height = ST7735_TFTHEIGHT
};

void spi_enable(void) {
    regbit_set_up(SPCR, SPE);
}

void spi_disable(void) {
    regbit_set_down(SPCR, SPE);
}

void spi_init(void) {
    /* Set PIN_MISO as input */
    regbit_set_down(DDRB, PIN_MISO);

    /* Set PIN_SS as output */
    regbit_set_up(DDRB, PIN_SS);
    regbit_set_up(DDRB, PIN_MOSI);
    regbit_set_up(DDRB, PIN_SCK);
    regbit_set_up(DDRB, PIN_RESET);
    regbit_set_up(DDRB, PIN_A0);

    /* Down SS */
    regbit_set_up(PORTB, PIN_SS);

    /* Set master mode */
    regbit_set_up(SPCR, MSTR);
    /* Set speed */
    regbit_set_down(SPCR, SPR0);
    regbit_set_down(SPCR, SPR1);
    /* Set duble speed */
    regbit_set_up(SPCR, SPI2X);

    /* Set order */
    regbit_set_down(SPCR, DORD);

    /* Set mode */
    regbit_set_down(SPCR, CPOL);
    regbit_set_down(SPCR, CPHA);

    spi_enable();
}

void spi_write_byte(uint8_t c) {
    SPDR = c;
    while (!(SPSR & (1 << SPIF)));
}

void spi_write_word(uint16_t w) {
    spi_write_byte(w >> 8);
    spi_write_byte(w & 0xff);
}

void spi_write_word_array(uint16_t w, uint16_t n) {
    while (n--)
        spi_write_word(w);
}


void lcd_write_command(uint8_t c) {
    regbit_set_down(PORTB, PIN_A0);
    regbit_set_down(PORTB, PIN_SS);

    spi_write_byte(c);

    regbit_set_up(PORTB, PIN_A0);
}

void lcd_write_byte(uint8_t c) {
    regbit_set_up(PORTB, PIN_A0);
    regbit_set_down(PORTB, PIN_SS);

    spi_write_byte(c);

    regbit_set_up(PORTB, PIN_A0);
}

void lcd_write_word(uint16_t w) {
    regbit_set_up(PORTB, PIN_A0);
    regbit_set_down(PORTB, PIN_SS);

    spi_write_word(w);

    regbit_set_up(PORTB, PIN_A0);
}


void lcd_write_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {

    lcd_addr_window(x, y, (x + w), (y + h));

    regbit_set_up(PORTB, PIN_A0);
    regbit_set_down(PORTB, PIN_SS);

    spi_write_word_array(color, (w + 1) * (h + 1));

    regbit_set_up(PORTB, PIN_A0);
}

void lcd_reset(void) {
    regbit_set_up(PORTB, PIN_RESET);
    _delay_ms(100);
    regbit_set_down(PORTB, PIN_RESET);
    _delay_ms(100);
    regbit_set_up(PORTB, PIN_RESET);
    _delay_ms(100);
}

void lcd_init(void) {

    //lcd_reset();
    regbit_set_up(PORTB, PIN_RESET);

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

#define LCD_DELTA_X     2
#define LCD_DELTA_Y     3

void lcd_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

    lcd_write_command(ST7735_CASET);
    lcd_write_word(x0 + LCD_DELTA_X);
    lcd_write_word(x1 + LCD_DELTA_X);

    lcd_write_command(ST7735_RASET);
    lcd_write_word(y0 + LCD_DELTA_Y);
    lcd_write_word(y1 + LCD_DELTA_Y);

    lcd_write_command(ST7735_RAMWR);
}

void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x > lcd_screen.height || y > lcd_screen.width)
        return;
    lcd_addr_window(x, y, x, y);
    lcd_write_word(color);
}

#define swap(a, b) { int16_t t = a; a = b; b = t; }

void lcd_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    while (x0 <= x1) {
        if (steep) {
            lcd_draw_pixel(y0, x0, color);
        } else {
            lcd_draw_pixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
        x0++;
    }
}

void lcd_draw_vline(int16_t x, int16_t y, int16_t l, uint16_t color) {
    lcd_write_rect(x, y, (l - 1), 0, color);
}

void lcd_draw_hline(int16_t x, int16_t y, int16_t l, uint16_t color) {
    lcd_write_rect(x, y, 0, (l - 1), color);
}

void lcd_draw_rest(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t color) {
    lcd_draw_hline(x1, y1, w, color);
    lcd_draw_vline(x1, y1, h, color);
    lcd_draw_hline((x1 + h) - 0, y1, w, color);
    lcd_draw_vline(x1, (y1 + w) - 0, h, color);

}

uint16_t lcd_rgb2color(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void lcd_orient0(void) {
    lcd_write_command(ST7735_MADCTL);
    lcd_write_byte(MADCTL_RGB);
}

void lcd_draw_char(uint16_t xbase, uint16_t ybase, font_t *font, uint8_t c) {
    if (c < font->start || c > (font->start + font->length))
        c = ' ';
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

void lcd_clear(void) {
    lcd_write_rect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, 0x0000);
}

/* EOF */
