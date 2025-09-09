#ifndef OLED_H_
#define OLED_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/* ======= ATtiny1614 + SPI0 (hardware) =======

   SPI0 default location on PORTA:
     MOSI = PA1
     SCK  = PA3
     MISO = PA2 (unused)

   Control pins (you can change to any available I/O):
*/
#define OLED_DC_PORT   PORTB
#define OLED_DC_DIR    PORTB.DIR
#define OLED_DC_OUT    PORTB.OUT
#define OLED_DC_bm     PIN2_bm   // PB2 -> DC

#define OLED_CS_PORT   PORTB
#define OLED_CS_DIR    PORTB.DIR
#define OLED_CS_OUT    PORTB.OUT
#define OLED_CS_bm     PIN3_bm   // PB3 -> CS

#define OLED_RES_PORT  PORTA
#define OLED_RES_DIR   PORTA.DIR
#define OLED_RES_OUT   PORTA.OUT
#define OLED_RES_bm    PIN4_bm   // PA4 -> RES

/* Display geometry */
#define OLED_WIDTH     128
#define OLED_HEIGHT     64
#define OLED_PAGES      (OLED_HEIGHT/8)

/* Framebuffer: 1bpp, 1024 bytes */
extern uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

/* API */
void oled_init(void);
void oled_clear(uint8_t color);                  // 0x00 black, 0xFF white
void oled_display(void);                         // flush framebuffer

/* Drawing */
void oled_draw_pixel(uint8_t x, uint8_t y, uint8_t color); // 0 or 1
void oled_draw_hline(uint8_t x, uint8_t y, uint8_t w, uint8_t color);
void oled_draw_vline(uint8_t x, uint8_t y, uint8_t h, uint8_t color);
void oled_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

/* Text (5x7 + 1px spacing = 6x8 cells) */
void oled_set_cursor(uint8_t col, uint8_t row);
void oled_write_char(char ch, uint8_t color);
void oled_print(const char* s, uint8_t color);

/* Extras */
void oled_invert(bool on);
void oled_contrast(uint8_t val);                 // 0..255

/* Number helpers (small, no stdio) */
void oled_print_int(int32_t v);
void oled_print_uint(uint32_t v);
void oled_print_fixed(int32_t scaled, uint8_t decimals); // 2537,2 => "25.37"
/* Pixel cursor (for big fonts) */
void oled_set_cursor_px(uint8_t x, uint8_t y);

/* Big text rendering from 5x7 font (scaled by 2 or 3) */
void oled_write_char_big(char ch, uint8_t scale, uint8_t color);   // scale: 2 or 3
void oled_print_big(const char* s, uint8_t scale, uint8_t color);  // '\n' moves to next line


#endif /* OLED_H_ */
