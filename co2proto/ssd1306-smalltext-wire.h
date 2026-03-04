
#include "font/font.h"
#include <Wire.h>

#define SSD1306_ADDR 0x3C

void ssd1306_cmd(uint8_t cmd) {
  Wire.beginTransmission(SSD1306_ADDR);
  Wire.write(0x00);
  Wire.write(cmd);
  Wire.endTransmission();
}

void ssd1306_init() {

  ssd1306_cmd(0xAE); // screen off

  ssd1306_cmd(0xD5);
  ssd1306_cmd(0x80); // Set osc frequency / clock divide ratio

  ssd1306_cmd(0xA8); // set mux ratio (1/64)
  // ssd1306_cmd(63);
  ssd1306_cmd(0x1f);

  // ssd1306_cmd(0xDA);
  // ssd1306_cmd(0x02); // COM pins config
  ssd1306_cmd(0xDA);
  ssd1306_cmd(0x02); // COM pins config: 128x64

  ssd1306_cmd(0xD3);
  ssd1306_cmd(0x00); // Display offset

  ssd1306_cmd(0x40 | 0); // Start line 0

  ssd1306_cmd(0xA0 | 1); // Segment remap

  // COM output scan direction
  ssd1306_cmd(0xC8); // Com Remapped — ekrāns normāls, ja usb pa kreisi
  // ssd1306_cmd(0xC0); // Com Normal — ekrāns invertēts, bet kaut kas griežas

  ssd1306_cmd(0x8D);
  ssd1306_cmd(0x14); // Enable charge pump regulator

  ssd1306_cmd(0x81);
  ssd1306_cmd(0x7F); // Contrast

  ssd1306_cmd(0xD9); // Pre-charge?
  ssd1306_cmd(0xF1);

  ssd1306_cmd(0xDB); // VCOM detect level?
  ssd1306_cmd(0x40);

  ssd1306_cmd(0xA4); // Resume display
  ssd1306_cmd(0xA6); // Normal display

  ssd1306_cmd(0x20); // addressing mode
  ssd1306_cmd(0b10); // Memory mode: paged
  // ssd1306_cmd(0b00); // Memory mode: horizontal

  ssd1306_cmd(0xAf); // Display on
}

void ssd1306_blit_page(uint8_t page, uint8_t *bufptr) {

  ssd1306_cmd(0x22); // Set page
  ssd1306_cmd(page);
  ssd1306_cmd(page);

  ssd1306_cmd(0x21); // Set page
  ssd1306_cmd(0);
  ssd1306_cmd(127);

  for (int nch = 0; nch < 16; nch++) {
    Wire.beginTransmission(SSD1306_ADDR);
    Wire.write(0x40); // Co = 0, D/C# = 1 (data)
    char c = *bufptr;
    bufptr++;
    uint8_t *char_ptr = Font + (c == 0 ? 0 : c - 32) * 8;
    Wire.write(*(char_ptr)++);
    Wire.write(*(char_ptr)++);
    Wire.write(*(char_ptr)++);
    Wire.write(*(char_ptr)++);
    Wire.write(*(char_ptr)++);
    Wire.write(*(char_ptr)++);
    Wire.write(*(char_ptr)++);
    Wire.write(*(char_ptr)++);
    Wire.endTransmission();
  }
}

uint8_t text_buffer[4][16];

void textn(int line, const char *msg) {
  int i = 0;
  for (; i < 16 && msg[i]; i++) {
    text_buffer[line][i] = msg[i];
  }
  for (; i < 16; i++) {
    text_buffer[line][i] = 0;
  }
}

void text0(const char *msg) {
  textn(0, msg);
  ssd1306_blit_page(0, text_buffer[0]);
}
void text1(const char *msg) {
  textn(1, msg);
  ssd1306_blit_page(1, text_buffer[1]);
}
void text2(const char *msg) {
  textn(2, msg);
  ssd1306_blit_page(2, text_buffer[2]);
}
void text3(const char *msg) {
  textn(3, msg);
  ssd1306_blit_page(3, text_buffer[3]);
}
