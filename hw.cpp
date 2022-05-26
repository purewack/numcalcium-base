#include "hw.h"
#include <libmaple/libmaple_types.h>
#include <libmaple/timer.h>
#include <libmaple/dma.h>
#include <libmaple/gpio.h>
#include "boards.h"
#include "io.h"

hw_t io;
soft_i2s_t i2s;
audio_buf_t abuf;

void io_mux_init(){
  if(io.inited) return;
  io.inited = 1;
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

  //48M / 48 / 2000 = 25hz*20keys
  timer_set_prescaler(TIMER3, 48);
  timer_set_reload(TIMER3, 1000);
  timer_attach_interrupt(TIMER3, TIMER_UPDATE_INTERRUPT, io_mux_irq);
  timer_enable_irq(TIMER3, TIMER_UPDATE_INTERRUPT);
  timer_resume(TIMER3);
}

void io_mux_irq(){

  if(io.op == 0){
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 1);
    io.op = 1;
    return;
  }

  else if(io.op == 1){
    uint32_t a = GPIOB->regs->IDR;
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 0);
    uint32_t ii = io.row*4;
    uint32_t readbyte = ((a&0x800)>>11) | ((a&0x400)>>9) | ((a&0x2)<<1) | ((a&0x1)<<3);
    io.bstate_old = io.bstate;
    io.bstate &= ~(0xf<<ii);
    io.bstate |= (readbyte<<ii);
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

void soft_i2s_init(){
  if(i2s.inited) return;
  i2s.inited = 1;
  abuf.buf_i = 0;

  disableDebugPorts();
  //timer4ch2 PB7 BCK
  gpio_set_mode(GPIOB, 15, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 12, GPIO_OUTPUT_PP);
  gpio_set_mode(GPIOB, 13, GPIO_AF_OUTPUT_PP);
  timer_pause(TIMER1);
  timer_set_prescaler(TIMER1, 0);
  timer_set_compare(TIMER1, TIMER_CH1, 24-1);
  timer_set_reload(TIMER1, 48-1);
  timer_dma_enable_req(TIMER1, TIMER_CH1);
  (TIMER1->regs.adv)->CCER |= 0b101;
 
  dma_init(DMA1);
  dma_disable(DMA1, DMA_CH2);
  int m = DMA_TRNS_CMPLT | DMA_HALF_TRNS | DMA_FROM_MEM | DMA_CIRC_MODE | DMA_MINC_MODE;
  dma_setup_transfer(DMA1, DMA_CH2 , (void*)&(GPIOB->regs->BSRR), DMA_SIZE_32BITS, i2s.dout_bits, DMA_SIZE_32BITS, m);
  dma_set_num_transfers(DMA1, DMA_CH2, 32);  
  dma_set_priority(DMA1, DMA_CH2, DMA_PRIORITY_HIGH);
  dma_attach_interrupt(DMA1, DMA_CH2, soft_i2s_bits_irq);
  dma_enable(DMA1, DMA_CH2);

  timer_resume(TIMER1);
}

void soft_i2s_deinit(){
  i2s.inited = 0;
  
  timer_pause(TIMER1);
  dma_detach_interrupt(DMA1, DMA_CH2);
  timer_dma_disable_trg_req(TIMER1);
}
// #define COMMS_MOSI PB15 //DOUT - bsr.15.31
// #define COMMS_MISO PB14 
// #define COMMS_CK   PB13 
// #define COMMS_CS PB12   //WS - bsr.12.28
void soft_i2s_bits_irq(){
  auto r = dma_get_irq_cause(DMA1, DMA_CH2) == DMA_TRANSFER_COMPLETE ? 1 : 0;
  auto rr = 16*r;
  auto ws = 0x90000000 | (0x1000*r);
  auto s = abuf.buf[abuf.buf_i];
  i2s.dout_bits[0+rr] = (( s & (0x8000>>0))<<0) | ws;
  i2s.dout_bits[1+rr] = (( s & (0x8000>>1))<<1) | ws;
  i2s.dout_bits[2+rr] = (( s & (0x8000>>2))<<2) | ws;
  i2s.dout_bits[3+rr] = (( s & (0x8000>>3))<<3) | ws;

  i2s.dout_bits[4+rr] = (( s & (0x8000>>4))<<4) | ws;
  i2s.dout_bits[5+rr] = (( s & (0x8000>>5))<<5) | ws;
  i2s.dout_bits[6+rr] = (( s & (0x8000>>6))<<6) | ws;
  i2s.dout_bits[7+rr] = (( s & (0x8000>>7))<<7) | ws;
  
  i2s.dout_bits[8 +rr] = (( s & (0x8000>>8 ))<<8 ) | ws;
  i2s.dout_bits[9 +rr] = (( s & (0x8000>>9 ))<<9 ) | ws;
  i2s.dout_bits[10+rr] = (( s & (0x8000>>10))<<10) | ws;
  i2s.dout_bits[11+rr] = (( s & (0x8000>>11))<<11) | ws;
  
  i2s.dout_bits[12+rr] = (( s & (0x8000>>12))<<12) | ws;
  i2s.dout_bits[13+rr] = (( s & (0x8000>>13))<<13) | ws;
  i2s.dout_bits[14+rr] = (( s & (0x8000>>14))<<14) | ws;
  i2s.dout_bits[15+rr] = (( s & (0x8000>>15))<<15) | ws;

  if(abuf.buf_i+1 == abuf.buf_len) abuf.req = 2;
  if(abuf.buf_i+1 == abuf.buf_len>>1) abuf.req = 1;
  abuf.buf_i = (abuf.buf_i+1)%abuf.buf_len;
}


// void pwm_audio_init(){

// }
// void pwm_audio_deinit(){

// }
// void pwm_audio_irq(){

// }