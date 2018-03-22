
#ifndef LCD_ST7735R_H 
#define LCD_ST7735R_H


#define ST7735_SWRESET  0x01    /* Software Reset */
#define ST7735_RDDID    0x04    /* Read Display ID */
#define ST7735_RDDST    0x09    /* Read Display Status */
#define ST7735_RDDPM    0x0A    /* Read Display Power Mode */
#define ST7735_RDDMADCTL        0x0B    /* Read Display MADCTL */
#define ST7735_RDDCOLMOD        0x0C    /* Read Display Pixel Format */
#define ST7735_RDDIM    0x0D    /* Read Display Image Mode */
#define ST7735_RDDSM    0x0E    /* Read Display Signal Mode */
#define ST7735_SLPIN    0x10    /* Sleep In */
#define ST7735_SLPOUT   0x11    /* Sleep Out */
#define ST7735_PTLON    0x12    /* Partial Display Mode On */
#define ST7735_NORON    0x13    /* Normal Display Mode On */
#define ST7735_INVOFF   0x20    /* Display Inversion Off */
#define ST7735_INVON    0x21    /* Display Inversion On */
#define ST7735_GAMSET   0x26    /* Gamma Set */
#define ST7735_DISPOFF  0x28    /* Display Off */
#define ST7735_DISPON   0x29    /* Display On */
#define ST7735_CASET    0x2A    /* Column Address Set */
#define ST7735_RASET    0x2B    /* Row Address Set */
#define ST7735_RAMWR    0x2C    /* Memory Write */
#define ST7735_RGBSET   0x2D    /* Color Setting for 4K, 65K and 262K */
#define ST7735_RAMRD    0x2E    /* Memory Read */
#define ST7735_PTLAR    0x30    /* Partial Area */
#define ST7735_TEOFF    0x34    /* Tearing Effect Line OFF */
#define ST7735_TEON     0x35    /* Tearing Effect Line ON */
#define ST7735_MADCTL   0x36    /* Memory Data Access Control */
#define ST7735_IDMOFF   0x38    /* Idle Mode Off */
#define ST7735_IDMON    0x39    /* Idle Mode On */
#define ST7735_COLMOD   0x3A    /* Interface Pixel Format */
#define ST7735_RDID1    0xDA    /* Read ID1 Value */
#define ST7735_RDID2    0xDB    /* Read ID2 Value */
#define ST7735_RDID3    0xDC    /* Read ID3 Value */
#define ST7735_FRMCTR1  0xB1    /* Frame Rate Control 0xIn normal mode/ Full colors) */
#define ST7735_FRMCTR2  0xB2    /* Frame Rate Control (In Idle mode/ 8-colors) */
#define ST7735_FRMCTR3  0xB3    /* Frame Rate Control 0xIn Partial mode/ full colors) */
#define ST7735_INVCTR   0xB4    /* Display Inversion Control */
#define ST7735_DISSET5  0xB6    /* Display Function set 5 */
#define ST7735_PWCTR1   0xC0    /* Power Control 1 */
#define ST7735_PWCTR2   0xC1    /* Power Control 2 */
#define ST7735_PWCTR3   0xC2    /* Power Control 3 0xin Normal mode/ Full colors) */
#define ST7735_PWCTR4   0xC3    /* Power Control 4 0xin Idle mode/ 8-colors) */
#define ST7735_PWCTR5   0xC4    /* Power Control 5 0xin Partial mode/ full-colors) */
#define ST7735_VMCTR1   0xC5    /* VCOM Control 1 */
#define ST7735_VMOFCTR  0xC7    /* VCOM Offset Control */
#define ST7735_WRID2    0xD1    /* Write ID2 Value */
#define ST7735_WRID3    0xD2    /* Write ID3 Value */
#define ST7735_NVFCTR1  0xD9    /* NVM Control Status */
#define ST7735_NVFCTR2  0xDE    /* NVM Read Command */
#define ST7735_NVFCTR3  0xDF    /* NVM Write Command */
#define ST7735_GMCTRP1  0xE0    /* Gamma 0x`+'polarity) Correction Characteristics Setting */
#define ST7735_GMCTRN1  0xE1    /* Gamma `-'polarity Correction Characteristics Setting */


#define ST7735_TFTWIDTH  127 //127
#define ST7735_TFTHEIGHT 159 //127

#define ST7735_BLACK     0x0000
#define ST7735_BLUE      0x001F
#define ST7735_RED       0xF800
#define ST7735_GREEN     0x07E0
#define ST7735_CYAN      0x07FF
#define ST7735_MAGENTA   0xF81F
#define ST7735_YELLOW    0xFFE0
#define ST7735_WHITE     0xFFFF


typedef struct font {
    uint8_t width;
    uint8_t height;
    uint8_t start;
    uint8_t length;
    const uint8_t *bitmap;
} font_t;

void spi_enable(void);
void spi_disable(void);
void spi_init(void);
void spi_write_byte(uint8_t c);
void spi_write_word(uint16_t w);
void spi_write_word_array(uint16_t w, uint16_t n);
void lcd_write_command(uint8_t c);
void lcd_write_byte(uint8_t c);
void lcd_write_word(uint16_t w);
void lcd_write_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void lcd_reset(void);
void lcd_init(void);
void lcd_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color);
void lcd_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void lcd_draw_vline(int16_t x, int16_t y, int16_t l, uint16_t color);
void lcd_draw_hline(int16_t x, int16_t y, int16_t l, uint16_t color);
void lcd_draw_rest(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
uint16_t lcd_rgb2color(uint8_t r, uint8_t g, uint8_t b);

void lcd_draw_char(uint16_t xbase, uint16_t ybase, font_t *font, uint8_t c);
void lcd_clear(void);
void lcd_orient0(void);
#endif
