#pragma once

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

// struct pwm_dac_t{
//   uint8_t inited = 0;
// };

void io_mux_init();
void io_mux_irq();

void soft_i2s_init();
void soft_i2s_deinit();
void soft_i2s_bits_irq();

void pwm_audio_init();
void pwm_audio_deinit();
void pwm_audio_irq();

void benchSetup();
void benchStart();
void benchEnd();