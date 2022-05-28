#include "hw.h"

#include <SPI.h>

hw_t io;
soft_i2s_t i2s;
audio_buf_t abuf;
lcd_t lcd;

void base_init(){
  disableDebugPorts();
  gpio_set_mode(GPIOB,5,GPIO_OUTPUT_PP);
  gpio_write_bit(GPIOB,5,0);
  
  gpio_set_mode(GPIOA,6,GPIO_AF_OUTPUT_PP);
}

void sys_power_down(){
  gpio_set_mode(GPIOB,5,GPIO_OUTPUT_PP);
  gpio_write_bit(GPIOB,5,1);
}

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
  timer_set_compare(TIMER3, TIMER_CH1, 0);
  timer_cc_enable(TIMER3, TIMER_CH1);
  timer_attach_interrupt(TIMER3, TIMER_UPDATE_INTERRUPT, io_mux_irq);
  timer_enable_irq(TIMER3, TIMER_UPDATE_INTERRUPT);
  timer_resume(TIMER3);
}

void io_mux_irq(){

  if(io.op == 0){
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 1);
    io.op = 1;
    io.lcd_fade_shadow += io.lcd_fade;
    if(io.lcd_fade_shadow <= io.lcd_low)  {
      io.lcd_fade = 0;
      io.lcd_fade_shadow = io.lcd_low;
    }
    if(io.lcd_fade_shadow >= io.lcd_hi-1) {
      io.lcd_fade = 0;
      io.lcd_fade_shadow = io.lcd_hi;
    }
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
    timer_set_compare(TIMER3, TIMER_CH1, io.lcd_fade_shadow);
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
//TIM4 CH 1 = PB6
//TIM4 CH 2 = PB7
//TIM1 CH2  = PA9
//TIM1 CH3  = PA10
//TIM2 CH2  = PA1
//https://i0.wp.com/blog.io-expert.com/wp-content/uploads/2019/08/clocks.png
//bit banging of WS, BCK, DATA on PB port pins 12,13,14 respectively
//GPIOB_BSSR register [0:15] set, [16:31] reset, non intrusive on other port pins - dma compatible

//either 
//  Timer_IRQ@srate -> (buf -> DMA -> GPIO_BSSR)
//or
//  DMA_IRQ(n)@halfbuf -> (buf -> Timer(n) -> Timer(n)CCMP_Pin)


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

void lcd_init(){
    SPI.begin();
    SPI.setBitOrder(MSBFIRST); // Set the SPI_1 bit order
    SPI.setDataMode(SPI_MODE0); //Set the  SPI_2 data mode 0
    SPI.setClockDivider(SPI_CLOCK_DIV8);      // Slow speed (72 / 16 = 4.5 MHz SPI_1 speed)

    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_RST, OUTPUT);
    pinMode(LCD_DC, OUTPUT);
    digitalWrite(LCD_CS, 1);
    digitalWrite(LCD_RST, 1);
    delay(1);
    digitalWrite(LCD_RST, 0);
    delay(1);
    digitalWrite(LCD_RST, 1);
    delay(100);
 
    //init
    digitalWrite(LCD_DC, 0);
    digitalWrite(LCD_CS, 0);
    
    SPI.transfer(0x0e2);                 /* soft reset */
    SPI.transfer(0x0ae);                    /* display off */
    SPI.transfer(0x040);                    /* set display start line to ... */
    
    SPI.transfer(0x0a1);                    /* ADC set to reverse */
    SPI.transfer(0x0c0);                    /* common output mode */
    
    SPI.transfer(0x0a6);                    /* display normal, bit val 0: LCD pixel off. */
    SPI.transfer(0x0a2);                    /* LCD bias 1/9 - *** Changed by Ismail - was 0xa3 - 1/7 bias we were getting dark pixel off */
    SPI.transfer(0x02f);                    /* all power  control circuits on (regulator, booster and follower) */
    
    SPI.transfer(0x0f8);
    SPI.transfer(0x000);    /* set booster ratio to 4x (ST7567 feature)*/
    
    SPI.transfer(0x027);                    /* set V0 voltage resistor ratio to max  */
    
    SPI.transfer(0x081);
    SPI.transfer(85);          /* set contrast, contrast value, EA default: 0x016 - *** Changed by Ismail to 0x05 */ 
    
    SPI.transfer(0x0af);                    /* display off */
    
    SPI.transfer(0b01000000); //line 0
    digitalWrite(LCD_CS, 1);
    
delay(100);

}

void lcd_update(){
  for(int p=0; p<8; p++){ 
    //page - col addy
    digitalWrite(LCD_DC, 0);
    digitalWrite(LCD_CS, 0);
    SPI.transfer(0b10110000 | p); //page
    SPI.transfer(0b00000100); //col 4
    SPI.transfer(0b00010000); //col high bit
    digitalWrite(LCD_CS, 1);
    delay(1);

    int pp = (p%4)*8;
    uint32_t* buf = p < 4 ? lcd.fbuf_top : lcd.fbuf_bot;

    digitalWrite(LCD_DC, 1);
    digitalWrite(LCD_CS, 0);
    for(int d=0; d<128; d++){
      //SPI.transfer(0b01010101 << (d%2)); //checkered
      uint32_t dd = buf[d];
      dd &= (0xff<<pp);
      SPI.transfer( uint8_t(dd>>pp) );
    }
    digitalWrite(LCD_CS, 1); 
    delay(1);
  }
}

void lcd_clear(){
  for(int i=0; i<128; i++){
    lcd.fbuf_top[i] = 0;
    lcd.fbuf_bot[i] = 0;
  }
}
void lcd_drawHline(int x, int y, int w){
  for(int i=x; i<x+w; i++){
    if(y<32)
      lcd.fbuf_top[i] |= (1<<y);
    else
      lcd.fbuf_bot[i] |= (1<<(y-32));
  }
}
void lcd_drawVline(int x, int y, int h){
  int e = y+h > 64 ? 64 : y+h;
  for(int i=y; i<e; i++){
    if(i<32)
      lcd.fbuf_top[x] |= (1<<(i));
    else
      lcd.fbuf_bot[x] |= (1<<(i-32));
  }
}

// void lcd_draw_rect_base(int x, int y, int w, int h, int fill){
//   lcd.fbuf[x] |= (((1<<h)-1)<<y);
//   int ff = fill ? ((1<<h)-1) : ((1<<(h-1)) | 1);
//   ff <<= y;
//   for(int i=x+1; i<x+w-1; i++){
//     lcd.fbuf[i] |= ff;
//   }
//   lcd.fbuf[x+w-1] |= (((1<<h)-1)<<y);
// }
void lcd_drawRectSize(int x, int y, int w, int h){
  //lcd_draw_rect_base(x,y,w,h,0);
  lcd_drawHline(x,y,w);
  lcd_drawVline(x,y,h);
  lcd_drawHline(x,y+h-1,w);
  lcd_drawVline(x+w-1,y,h);
}