#ifndef F_CPU
#define F_CPU 3333333UL
#endif

#include "oled.h"
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>

/* ======== Hardware SPI0 (ATtiny1614) ======== */
static inline void spi0_init(void) {
    // SPI0 default location on PORTA (POR TMUX.SPI0 = 0)
    // PA1 = MOSI (output), PA3 = SCK (output), PA2 = MISO (input)
    PORTA.DIRSET = PIN1_bm | PIN3_bm;
    PORTA.DIRCLR = PIN2_bm;

    // Enable SPI0 as Master, Prescaler /4 (you can go faster),
    // Mode 0 (CPOL=0, CPHA=0), MSB first
    SPI0.CTRLA = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_PRESC_DIV4_gc;
    SPI0.CTRLB = 0; // default
}

static inline void spi0_tx(uint8_t b) {
    SPI0.DATA = b;
    while (!(SPI0.INTFLAGS & SPI_IF_bm)) { ; }
}

/* ======== SSD1306 low-level ======== */
static inline void dc_cmd(void)   { OLED_DC_OUT &= ~OLED_DC_bm; }
static inline void dc_data(void)  { OLED_DC_OUT |=  OLED_DC_bm; }
static inline void cs_low(void)   { OLED_CS_OUT &= ~OLED_CS_bm; }
static inline void cs_high(void)  { OLED_CS_OUT |=  OLED_CS_bm; }

static inline void ssd1306_cmd(uint8_t c) {
    dc_cmd(); cs_low(); spi0_tx(c); cs_high();
}

static inline void ssd1306_data_bytes(const uint8_t* p, uint16_t n) {
    dc_data(); cs_low();
    while (n--) spi0_tx(*p++);
    cs_high();
}

static inline void set_col_range(uint8_t x0, uint8_t x1) {
    ssd1306_cmd(0x21); ssd1306_cmd(x0); ssd1306_cmd(x1);
}
static inline void set_page_range(uint8_t p0, uint8_t p1) {
    ssd1306_cmd(0x22); ssd1306_cmd(p0); ssd1306_cmd(p1);
}

/* ======== Framebuffer ======== */
uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

/* ======== 5x7 Font (ASCII 0x20..0x7E) ======== */
static const uint8_t font5x7[] PROGMEM = {
    0x00,0x00,0x00,0x00,0x00, /* ' ' */
    0x00,0x00,0x5F,0x00,0x00, /* '!' */
    0x00,0x07,0x00,0x07,0x00, /* '"' */
    0x14,0x7F,0x14,0x7F,0x14, /* '#' */
    0x24,0x2A,0x7F,0x2A,0x12, /* '$' */
    0x23,0x13,0x08,0x64,0x62, /* '%' */
    0x36,0x49,0x55,0x22,0x50, /* '&' */
    0x00,0x05,0x03,0x00,0x00, /* ''' */
    0x00,0x1C,0x22,0x41,0x00, /* '(' */
    0x00,0x41,0x22,0x1C,0x00, /* ')' */
    0x14,0x08,0x3E,0x08,0x14, /* '*' */
    0x08,0x08,0x3E,0x08,0x08, /* '+' */
    0x00,0x50,0x30,0x00,0x00, /* ',' */
    0x08,0x08,0x08,0x08,0x08, /* '-' */
    0x00,0x60,0x60,0x00,0x00, /* '.' */
    0x20,0x10,0x08,0x04,0x02, /* '/' */
    0x3E,0x51,0x49,0x45,0x3E, /* '0' */
    0x00,0x42,0x7F,0x40,0x00, /* '1' */
    0x42,0x61,0x51,0x49,0x46, /* '2' */
    0x21,0x41,0x45,0x4B,0x31, /* '3' */
    0x18,0x14,0x12,0x7F,0x10, /* '4' */
    0x27,0x45,0x45,0x45,0x39, /* '5' */
    0x3C,0x4A,0x49,0x49,0x30, /* '6' */
    0x01,0x71,0x09,0x05,0x03, /* '7' */
    0x36,0x49,0x49,0x49,0x36, /* '8' */
    0x06,0x49,0x49,0x29,0x1E, /* '9' */
    0x00,0x36,0x36,0x00,0x00, /* ':' */
    0x00,0x56,0x36,0x00,0x00, /* ';' */
    0x08,0x14,0x22,0x41,0x00, /* '<' */
    0x14,0x14,0x14,0x14,0x14, /* '=' */
    0x00,0x41,0x22,0x14,0x08, /* '>' */
    0x02,0x01,0x51,0x09,0x06, /* '?' */
    0x3E,0x41,0x5D,0x59,0x4E, /* '@' */
    0x7E,0x11,0x11,0x11,0x7E, /* 'A' */
    0x7F,0x49,0x49,0x49,0x36, /* 'B' */
    0x3E,0x41,0x41,0x41,0x22, /* 'C' */
    0x7F,0x41,0x41,0x22,0x1C, /* 'D' */
    0x7F,0x49,0x49,0x49,0x41, /* 'E' */
    0x7F,0x09,0x09,0x09,0x01, /* 'F' */
    0x3E,0x41,0x49,0x49,0x7A, /* 'G' */
    0x7F,0x08,0x08,0x08,0x7F, /* 'H' */
    0x00,0x41,0x7F,0x41,0x00, /* 'I' */
    0x20,0x40,0x41,0x3F,0x01, /* 'J' */
    0x7F,0x08,0x14,0x22,0x41, /* 'K' */
    0x7F,0x40,0x40,0x40,0x40, /* 'L' */
    0x7F,0x02,0x0C,0x02,0x7F, /* 'M' */
    0x7F,0x04,0x08,0x10,0x7F, /* 'N' */
    0x3E,0x41,0x41,0x41,0x3E, /* 'O' */
    0x7F,0x09,0x09,0x09,0x06, /* 'P' */
    0x3E,0x41,0x51,0x21,0x5E, /* 'Q' */
    0x7F,0x09,0x19,0x29,0x46, /* 'R' */
    0x46,0x49,0x49,0x49,0x31, /* 'S' */
    0x01,0x01,0x7F,0x01,0x01, /* 'T' */
    0x3F,0x40,0x40,0x40,0x3F, /* 'U' */
    0x1F,0x20,0x40,0x20,0x1F, /* 'V' */
    0x7F,0x20,0x18,0x20,0x7F, /* 'W' */
    0x63,0x14,0x08,0x14,0x63, /* 'X' */
    0x07,0x08,0x70,0x08,0x07, /* 'Y' */
    0x61,0x51,0x49,0x45,0x43, /* 'Z' */
    0x00,0x7F,0x41,0x41,0x00, /* '[' */
    0x02,0x04,0x08,0x10,0x20, /* '\' */
    0x00,0x41,0x41,0x7F,0x00, /* ']' */
    0x04,0x02,0x01,0x02,0x04, /* '^' */
    0x40,0x40,0x40,0x40,0x40, /* '_' */
    0x00,0x03,0x07,0x00,0x00, /* '`' */
    0x20,0x54,0x54,0x54,0x78, /* 'a' */
    0x7F,0x48,0x44,0x44,0x38, /* 'b' */
    0x38,0x44,0x44,0x44,0x20, /* 'c' */
    0x38,0x44,0x44,0x48,0x7F, /* 'd' */
    0x38,0x54,0x54,0x54,0x18, /* 'e' */
    0x08,0x7E,0x09,0x01,0x02, /* 'f' */
    0x0C,0x52,0x52,0x52,0x3E, /* 'g' */
    0x7F,0x08,0x04,0x04,0x78, /* 'h' */
    0x00,0x44,0x7D,0x40,0x00, /* 'i' */
    0x20,0x40,0x44,0x3D,0x00, /* 'j' */
    0x7F,0x10,0x28,0x44,0x00, /* 'k' */
    0x00,0x41,0x7F,0x40,0x00, /* 'l' */
    0x7C,0x04,0x18,0x04,0x78, /* 'm' */
    0x7C,0x08,0x04,0x04,0x78, /* 'n' */
    0x38,0x44,0x44,0x44,0x38, /* 'o' */
    0x7C,0x14,0x14,0x14,0x08, /* 'p' */
    0x08,0x14,0x14,0x14,0x7C, /* 'q' */
    0x7C,0x08,0x04,0x04,0x08, /* 'r' */
    0x48,0x54,0x54,0x54,0x20, /* 's' */
    0x04,0x3F,0x44,0x40,0x20, /* 't' */
    0x3C,0x40,0x40,0x20,0x7C, /* 'u' */
    0x1C,0x20,0x40,0x20,0x1C, /* 'v' */
    0x3C,0x40,0x30,0x40,0x3C, /* 'w' */
    0x44,0x28,0x10,0x28,0x44, /* 'x' */
    0x0C,0x50,0x50,0x50,0x3C, /* 'y' */
    0x44,0x64,0x54,0x4C,0x44  /* 'z' */
};

/* ======== Reset + init ======== */
static void ssd1306_reset(void) {
    OLED_RES_DIR |= OLED_RES_bm;
    OLED_RES_OUT |= OLED_RES_bm; _delay_ms(1);
    OLED_RES_OUT &= ~OLED_RES_bm; _delay_ms(5);
    OLED_RES_OUT |= OLED_RES_bm; _delay_ms(5);
}

static void ssd1306_init_sequence(void) {
    ssd1306_cmd(0xAE);                      // display off
    ssd1306_cmd(0xD5); ssd1306_cmd(0x80);   // clock div
    ssd1306_cmd(0xA8); ssd1306_cmd(0x3F);   // multiplex 1/64
    ssd1306_cmd(0xD3); ssd1306_cmd(0x00);   // display offset
    ssd1306_cmd(0x40 | 0x00);               // start line 0
    ssd1306_cmd(0x8D); ssd1306_cmd(0x14);   // charge pump on
    ssd1306_cmd(0x20); ssd1306_cmd(0x00);   // horizontal addressing
    ssd1306_cmd(0xA1);                      // seg remap
    ssd1306_cmd(0xC8);                      // COM scan dec
    ssd1306_cmd(0xDA); ssd1306_cmd(0x12);   // COM pins
    ssd1306_cmd(0x81); ssd1306_cmd(0x7F);   // contrast
    ssd1306_cmd(0xD9); ssd1306_cmd(0xF1);   // precharge
    ssd1306_cmd(0xDB); ssd1306_cmd(0x40);   // VCOM detect
    ssd1306_cmd(0xA4);                      // display from RAM
    ssd1306_cmd(0xA6);                      // normal
    ssd1306_cmd(0xAF);                      // display on
}

/* ======== Public API ======== */
void oled_init(void) {
    // DC/CS outputs
    OLED_DC_DIR  |= OLED_DC_bm;  OLED_DC_OUT &= ~OLED_DC_bm;
    OLED_CS_DIR  |= OLED_CS_bm;  OLED_CS_OUT |=  OLED_CS_bm; // CS idle high

    spi0_init();
    ssd1306_reset();
    ssd1306_init_sequence();

    oled_clear(0x00);
    oled_display();
}

void oled_clear(uint8_t color) {
    memset(oled_buffer, color ? 0xFF : 0x00, sizeof(oled_buffer));
}

void oled_invert(bool on) {
    ssd1306_cmd(on ? 0xA7 : 0xA6);
}

void oled_contrast(uint8_t val) {
    ssd1306_cmd(0x81); ssd1306_cmd(val);
}

/* ======== Flush framebuffer ======== */
void oled_display(void) {
    set_col_range(0, OLED_WIDTH - 1);
    set_page_range(0, (OLED_PAGES - 1));
    ssd1306_data_bytes(oled_buffer, sizeof(oled_buffer));
}

/* ======== Drawing ======== */
void oled_draw_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint16_t i = x + (y >> 3) * OLED_WIDTH;
    uint8_t  m = (uint8_t)(1u << (y & 7));
    if (color) oled_buffer[i] |=  m;
    else       oled_buffer[i] &= ~m;
}

void oled_draw_hline(uint8_t x, uint8_t y, uint8_t w, uint8_t color) {
    if (y >= OLED_HEIGHT || x >= OLED_WIDTH || w == 0) return;
    if (x + w > OLED_WIDTH) w = OLED_WIDTH - x;
    for (uint8_t i = 0; i < w; i++) oled_draw_pixel(x + i, y, color);
}

void oled_draw_vline(uint8_t x, uint8_t y, uint8_t h, uint8_t color) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT || h == 0) return;
    if (y + h > OLED_HEIGHT) h = OLED_HEIGHT - y;
    for (uint8_t i = 0; i < h; i++) oled_draw_pixel(x, y + i, color);
}

void oled_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT || w == 0 || h == 0) return;
    if (x + w > OLED_WIDTH) w = OLED_WIDTH - x;
    if (y + h > OLED_HEIGHT) h = OLED_HEIGHT - y;
    for (uint8_t yy = 0; yy < h; yy++) oled_draw_hline(x, y + yy, w, color);
}

/* ======== Text (6x8 cell) ======== */
static uint8_t cur_col = 0, cur_row = 0;

void oled_set_cursor(uint8_t col, uint8_t row) {
    if (col >= (OLED_WIDTH/6)) col = (OLED_WIDTH/6)-1;
    if (row >= (OLED_HEIGHT/8)) row = (OLED_HEIGHT/8)-1;
    cur_col = col; cur_row = row;
}

void oled_write_char(char ch, uint8_t color) {
    if (ch < 0x20 || ch > 0x7E) ch = '?';
    uint16_t idx = (uint16_t)(ch - 0x20) * 5;

    uint8_t x0 = cur_col * 6;
    uint8_t y0 = cur_row * 8;

    // 5 glyph columns
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t colbits = pgm_read_byte(&font5x7[idx + col]);
        for (uint8_t row = 0; row < 7; row++) {
            uint8_t on = (colbits >> row) & 1u;
            oled_draw_pixel(x0 + col, y0 + row, on ? color : 0);
        }
    }
    // 1px spacing column cleared
    for (uint8_t row = 0; row < 7; row++) oled_draw_pixel(x0 + 5, y0 + row, 0);

    // advance
    cur_col++;
    if (cur_col >= (OLED_WIDTH / 6)) {
        cur_col = 0;
        cur_row++;
        if (cur_row >= (OLED_HEIGHT / 8)) cur_row = 0;
    }
}

void oled_print(const char* s, uint8_t color) {
    while (*s) {
        if (*s == '\n') { cur_col = 0; cur_row = (cur_row + 1) % (OLED_HEIGHT/8); s++; continue; }
        oled_write_char(*s++, color);
    }
}

/* ======== Number printing (no stdio) ======== */
static void _print_uint(uint32_t v) {
    char buf[11]; uint8_t i = 0;
    if (v == 0) { oled_write_char('0', 1); return; }
    while (v && i < sizeof(buf)) { buf[i++] = '0' + (v % 10u); v /= 10u; }
    while (i--) oled_write_char(buf[i], 1);
}
void oled_print_uint(uint32_t v) { _print_uint(v); }

void oled_print_int(int32_t v) {
    if (v < 0) { oled_write_char('-',1); uint32_t mag = (uint32_t)(-(v+1)) + 1u; _print_uint(mag); }
    else _print_uint((uint32_t)v);
}

void oled_print_fixed(int32_t scaled, uint8_t decimals) {
    bool neg = (scaled < 0);
    uint32_t mag = neg ? (uint32_t)(-(scaled+1)) + 1u : (uint32_t)scaled;
    uint32_t pow10 = 1; for (uint8_t i=0;i<decimals;i++) pow10 *= 10u;
    uint32_t ip = mag / pow10, fp = mag % pow10;
    if (neg) oled_write_char('-',1);
    _print_uint(ip);
    if (decimals) {
        oled_write_char('.',1);
        // print fp with leading zeros
        char tmp[10]; for (int8_t i=decimals-1;i>=0;i--){ tmp[i] = '0' + (fp % 10u); fp/=10u; }
        for (uint8_t i=0;i<decimals;i++) oled_write_char(tmp[i],1);
    }
}
/* ---------- Pixel cursor for big text ---------- */
static uint8_t cur_x_px = 0, cur_y_px = 0;

void oled_set_cursor_px(uint8_t x, uint8_t y) {
    if (x >= OLED_WIDTH)  x = OLED_WIDTH - 1;
    if (y >= OLED_HEIGHT) y = OLED_HEIGHT - 1;
    cur_x_px = x;
    cur_y_px = y;
}

/* Draw one scaled 5x7 glyph at (cur_x_px, cur_y_px).
   scale = 2 -> ~12x16 cell (5*2 + 1 spacing, 7*2)
   scale = 3 -> ~18x24 cell (5*3 + 1 spacing, 7*3) */
void oled_write_char_big(char ch, uint8_t scale, uint8_t color) {
    if (scale < 2) scale = 2;
    if (scale > 3) scale = 3;

    if (ch < 0x20 || ch > 0x7E) ch = '?';
    uint16_t idx = (uint16_t)(ch - 0x20) * 5;

    uint8_t x0 = cur_x_px;
    uint8_t y0 = cur_y_px;

    // Draw 5 columns of glyph
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t colbits = pgm_read_byte(&font5x7[idx + col]);
        // For each of 7 rows
        for (uint8_t row = 0; row < 7; row++) {
            if ((colbits >> row) & 1u) {
                // Filled block scale x scale
                uint8_t px = x0 + col * scale;
                uint8_t py = y0 + row * scale;
                for (uint8_t yy = 0; yy < scale; yy++) {
                    for (uint8_t xx = 0; xx < scale; xx++) {
                        oled_draw_pixel(px + xx, py + yy, color);
                    }
                }
            }
        }
    }

    // spacing column (scale pixels wide), clear to background (color=0)
    for (uint8_t row = 0; row < 7 * scale; row++) {
        for (uint8_t xx = 0; xx < scale; xx++) {
            oled_draw_pixel(x0 + 5 * scale + xx, y0 + row, 0);
        }
    }

    // Advance pixel cursor
    cur_x_px += (uint8_t)(5 * scale + scale);   // 5 scaled cols + scaled space
    // (No automatic wrap here; use oled_set_cursor_px or \n handling in print_big)
}

void oled_print_big(const char* s, uint8_t scale, uint8_t color) {
    uint8_t line_height = (uint8_t)(7 * scale);        // 14 for x2, 21 for x3
    uint8_t char_advance = (uint8_t)(5 * scale + scale);

    while (*s) {
        if (*s == '\n') {
            cur_x_px = 0;
            // next line with 1px gap
            uint16_t ny = (uint16_t)cur_y_px + line_height + 1;
            cur_y_px = (ny >= OLED_HEIGHT) ? 0 : (uint8_t)ny;
            s++;
            continue;
        }

        // If next char would overflow horizontally, wrap to next line
        if (cur_x_px + char_advance > OLED_WIDTH) {
            cur_x_px = 0;
            uint16_t ny = (uint16_t)cur_y_px + line_height + 1;
            cur_y_px = (ny >= OLED_HEIGHT) ? 0 : (uint8_t)ny;
        }

        oled_write_char_big(*s++, scale, color);
    }
}
