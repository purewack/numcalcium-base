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
  #define S_BIT(n,b) n |= (1<<(b-1))
  #define C_BIT(n,b) n &= ~(1<<(b-1))
  #define BUTTON(b) (1<<(b-1))
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
  timer_dev* mux_timer;
};


void io_mux_init()
void io_mux_irq()


void benchSetup(){
  //gpio_set_mode(GPIOB, 14, GPIO_OUTPUT_PP);
}

void benchStart(){
  //GPIOB->regs->BSRR = 1<<14;
}

void benchEnd(){
  //GPIOB->regs->BSRR = (1<<14)<<16;
}