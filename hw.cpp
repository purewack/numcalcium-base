#include "hw.h"
#include <libmaple/libmaple_types.h>
#include <libmaple/timer.h>
#include <libmaple/dma.h>
#include <libmaple/gpio.h>
#include "boards.h"
#include "io.h"

hw_t io;

void io_mux_init(){
  disableDebugPorts();
  gpio_set_mode(GPIOA, 14, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOA, 15, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 3, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 4, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 8, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 9, GPIO_OUTPUT_PP);

  gpio_set_mode(GPIOB, 11, GPIO_INPUT_PD);
  gpio_set_mode(GPIOB, 10, GPIO_INPUT_PD);
  gpio_set_mode(GPIOB, 1, GPIO_INPUT_PD);
  gpio_set_mode(GPIOB, 0, GPIO_INPUT_PD);

  gpio_set_mode(GPIOA, 8, GPIO_INPUT_PD);

  io.mux_timer = TIMER3;
  timer_pause(io.mux_timer);

  //48M / 48 / 2000 = 25hz*20keys
  timer_set_prescaler(io.mux_timer, 48);
  timer_set_reload(io.mux_timer, 1000);
  timer_attach_interrupt(io.mux_timer, TIMER_UPDATE_INTERRUPT, io_mux_irq);
  timer_enable_irq(io.mux_timer, TIMER_UPDATE_INTERRUPT);
  timer_resume(io.mux_timer);
}

void io_mux_irq(){

  if(io.op == 0){
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 1);
    io.op = 1;
    return;
  }

  else if(io.op == 1){
    auto a = GPIOB->regs->IDR;
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 0);

    auto readbyte = ((a&0xc00)>>8) | (a&0x3);
    io.bstate_old = io.bstate;
    io.bstate &= ~(0xf<<(io.row*4));
    io.bstate |= (readbyte<<(io.row*4));
    io.bscan_down |= (io.bstate & (~io.bstate_old));
    io.bscan_up |= (io.bstate_old & (~io.bstate));
    
    io.row = (io.row+1)%5;
    io.op = 2;
    return;
  }
  
  else if(io.op == 2){
    gpio_write_bit(GPIOB, 9, 1);
    io.ok = gpio_read_bit(GPIOA, 8);
    io.op = 3;
    return;
  }

  io.op = 0;
  // auto a = GPIOB->regs->IDR;
  // io.io[20].state = (a&(1<<11))>>11;
  // io.io[21].state = (a&(1<<10))>>10;
  // gpio_write_bit(GPIOB, 9, 0);

  // io.turns_state = (io.io[20].state<<0) | (io.io[21].state<<1) | (io.io[20].state_old<<2) | (io.io[21].state_old<<3);
  // io.io[20].state_old = io.io[20].state;
  // io.io[21].state_old = io.io[21].state;

  // if(io.turns_state == 0b0001){
  //   if(io.turns < 0) io.turns = 0;
  //   io.turns++;
  // }
  // else if(io.turns_state == 0b1101){
  //   if(io.turns > 0) io.turns = 0;
  //   io.turns--;
  // }
}
