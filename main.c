#ifndef F_CPU
#define F_CPU 3333333UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "oled.h"

int main(void) {
    oled_init();
  /*
    oled_clear(0x00);
    oled_set_cursor(0,0);
    oled_print("ATTINY1614 + SSD1306", 1);
    int val = 0;

    // simple graphics/*
   /* oled_draw_hline(0, 10, 128, 1);
    oled_draw_vline(0, 10, 54, 1);
    oled_fill_rect(100, 48, 28, 16, 1);

    // numbers
    oled_set_cursor(0,2);
    oled_print("TEMP:", 1);
    oled_print_fixed(2537, 2); // 25.37
    oled_write_char('C', 1);

    oled_set_cursor(0,3);
    oled_print("COUNT:", 1);
    oled_print_uint(12345);*/
    // Big title (x2)
oled_set_cursor_px(0, 0);
oled_print_big("TEMP", 2, 1);

// Big number (x3), e.g. 25.37
oled_set_cursor_px(0, 20);
oled_print_big("25.37C", 3, 1);

// Mixed: small labels + big values
oled_set_cursor(0, 6);                 // small 6x8 text row
oled_print("MIN:", 1);
oled_set_cursor_px(36, 48);
oled_print_big("20.1", 2, 1);

oled_display();

    oled_display();

    // tiny animation
    uint8_t x = 0;
    while (1) {
        // move a small bar
      /*  oled_fill_rect(10, 40, 20, 8, 0);
        oled_fill_rect(10 + x, 40, 20, 8, 1);
        oled_display();
        _delay_ms(30);
        x = (x + 3) % 98;*/
      /*  val ++;
        if(val>500)val = 0;
          oled_set_cursor(0,6);
    oled_print("value:=", 1);
    oled_print_uint(val);
    oled_display();
    _delay_ms(100);*/
        
    }
}
