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

  io.itr = 0;
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

  if(kmux.op == 0){
    gpio_write_bit(kmux.row < 2 ? GPIOA : GPIOB, kmux.seq_row[kmux.row], 1);
    kmux.op = 1;
    return;
  }

  else if(kmux.op == 1){
    auto a = GPIOB->regs->IDR;
    gpio_write_bit(kmux.row < 2 ? GPIOA : GPIOB, kmux.seq_row[kmux.row], 0);

    auto readbyte = ((a&0xc00)>>8) | (a&0x3);
    kmux.bstate_old = kmux.bstate;
    kmux.bstate &= ~(0xf<<(kmux.row*4));
    kmux.bstate |= (readbyte<<(kmux.row*4));
    kmux.bscan_down |= (kmux.bstate & (~kmux.bstate_old));
    kmux.bscan_up |= (kmux.bstate_old & (~kmux.bstate));
    
    kmux.row = (kmux.row+1)%5;
    kmux.op = 2;
    return;
  }
  
  else if(kmux.op == 2){
    gpio_write_bit(GPIOB, 9, 1);
    kmux.ok = gpio_read_bit(GPIOA, 8);
    kmux.op = 3;
    return;
  }

  kmux.op = 0;
  // auto a = GPIOB->regs->IDR;
  // kmux.io[20].state = (a&(1<<11))>>11;
  // kmux.io[21].state = (a&(1<<10))>>10;
  // gpio_write_bit(GPIOB, 9, 0);

  // kmux.turns_state = (kmux.io[20].state<<0) | (kmux.io[21].state<<1) | (kmux.io[20].state_old<<2) | (kmux.io[21].state_old<<3);
  // kmux.io[20].state_old = kmux.io[20].state;
  // kmux.io[21].state_old = kmux.io[21].state;

  // if(kmux.turns_state == 0b0001){
  //   if(kmux.turns < 0) kmux.turns = 0;
  //   kmux.turns++;
  // }
  // else if(kmux.turns_state == 0b1101){
  //   if(kmux.turns > 0) kmux.turns = 0;
  //   kmux.turns--;
  // }
}
