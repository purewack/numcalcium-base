#pragma once

#define SYS_PDOWN PB5

#define LCD_LIGHT PA6
#define LCD_CS PA4
#define LCD_DC PA3
#define LCD_CK PA5
#define LCD_RST PA2
#define LCD_MOSI PA7

#define SPI_CS 4
#define SPI_DC 3
#define SPI_RST 2
#define SPI_MOSI 7
#define SPI_CLOCK 5
#define LCD_LIGHT_PWM 6

#define SEG_A PB11
#define SEG_B PB10
#define SEG_C PB1
#define SEG_D PB0

#define ROW_A PA14
#define ROW_B PA15
#define ROW_C PB3
#define ROW_D PB4
#define ROW_E PB8
#define ROW_F PB9

#define B_OK PA8

#define GP_A PA0
#define GP_B PA1
#define COMMS_SDA PB7
#define COMMS_SDL PB6
#define COMMS_MOSI PB15
#define COMMS_MISO PB14
#define COMMS_CK   PB13
#define COMMS_CS PB12
#define COMMS_RX PA10
#define COMMS_TX PA9

#define K_Y 0
#define K_0 1
#define K_DOT 2
#define K_R 3
#define K_1 4
#define K_2 5
#define K_3 6
#define K_P 7
#define K_4 8
#define K_5 9
#define K_6 10
#define K_N 11
#define K_7 12
#define K_8 13
#define K_9 14
#define K_D 15
#define K_F1 16
#define K_F2 17
#define K_F3 18
#define K_X 19

#include <Arduino.h>
#include <libmaple/libmaple_types.h>
#include <libmaple/timer.h>
#include <libmaple/dma.h>
#include <libmaple/gpio.h>
#include <libmaple/spi.h>
#include <libmaple/delay.h>
#include "boards.h"
#include "io.h"

#include "fonttiny.h"

struct hw_t
{
  #define S_BIT(n,b) n |= (1<<(b))
  #define C_BIT(n,b) n &= ~(1<<(b))
  #define BUTTON(b) (1<<(b))
  uint32_t bstate;
  uint32_t bstate_old;
  uint32_t bscan_down;
  uint32_t bscan_up;
  uint8_t ok;
  int turns;
  int turns_old;
  int turns_state;
  int turns_state_old;
  const uint8_t seq_row[7] = {14,15,3,4,8};
  uint8_t row;
  uint8_t op;
  uint8_t inited = 0;
};

struct audio_buf_t {
  const uint8_t buf_len = 128;
  int16_t buf[128];
  uint8_t buf_i;
  uint8_t req = 0;
};
struct soft_i2s_t{
  uint32_t dout_bits[32];
  uint8_t inited = 0;
};

struct lcd_t {
  int inited = 0;
  volatile int transferring = 0;

  uint32_t fbuf_top[128]; // 128 vertical columns
  uint32_t fbuf_bot[128]; // 128 vertical columns
  
  const uint8_t init_seq[14] = {
    0xe2,                    /* soft reset */
    0xae,                    /* display off */
    0x40,                    /* display start line 0 */
    0xa1,                    /* ADC set to reverse */
    0xc0,                    /* common output mode */
    0xa6,                    /* display normal, bit val 0: LCD pixel off. */
    0xa2,                    /* LCD bias 1/9 */
    0x2f,                    /* all power */
    0xf8,
    0x00,    /* set booster ratio to 4x */
    0x27,                    /* set V0 voltage resistor ratio to max  */
    0x81,
    0x52,       /* set contrast, contrast value, 80/127 */ 
    0xaf,                    /* display on */
  };
  const uint8_t init_seq_len = 14;
  uint8_t data_seq[3] = {
    0,
    0x04,
    0x10
  };
};

extern hw_t io;
extern soft_i2s_t i2s;
extern audio_buf_t abuf;
extern lcd_t lcd;

struct font_t {
  uint8_t tall;
  uint8_t wide;
  void* data;
  uint16_t data_count;
};
// struct pwm_dac_t{
//   uint8_t inited = 0;
// };

void base_init();
void base_deinit();

void io_mux_init();
void io_mux_irq();

void soft_i2s_init();
void soft_i2s_deinit();
void soft_i2s_bits_irq();

void pwm_audio_init();
void pwm_audio_deinit();
void pwm_audio_irq();

void lcd_fade(int in);
void lcd_init();
void lcd_clear();
void lcd_clearSection(int yoff, int ylen, int xoff, int xlen, int fill);
void lcd_update();
void lcd_updateSection(int pageoffset, int pagelen, int offset, int len);
void lcd_drawHline(int x, int y, int w);
void lcd_drawVline(int x, int y, int h);
void lcd_drawLine(int x, int y, int x2, int y2);
void lcd_drawRectPoint(int x, int y, int x2, int y2);
void lcd_drawRectSize(int x, int y, int w, int h);
void lcd_fillRectSize(int x, int y, int w, int h, int pattern);
int lcd_drawChar(int x, int y, font_t font_info, char g);
int lcd_drawString(int x, int y, font_t font_info, const char* str);

void benchSetup();
void benchStart();
void benchEnd();
