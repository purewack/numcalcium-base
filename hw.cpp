#include "hw.h"

hw_t io;
soft_i2s_t i2s;
audio_buf_t abuf;
lcd_t lcd;

void base_init(){
  disableDebugPorts();
  gpio_write_bit(GPIOB,5,0);
  gpio_set_mode(GPIOB,5,GPIO_OUTPUT_PP);
  
  lcd_init();
  io_mux_init();
  lcd_clear();
  lcd_update();
}

void base_deinit(){
  gpio_write_bit(GPIOB,5,1);
}

void io_mux_init(){
  if(io.inited) return;
  io.inited = 1;
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
  timer_attach_interrupt(TIMER3, TIMER_UPDATE_INTERRUPT, io_mux_irq);
  timer_enable_irq(TIMER3, TIMER_UPDATE_INTERRUPT);
  timer_resume(TIMER3);
}

void io_mux_irq(){

  if(io.op == 0){
    io.ok |= gpio_read_bit(GPIOA, 8)>>8;
    gpio_write_bit(io.row < 2 ? GPIOA : GPIOB, io.seq_row[io.row], 1);
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
    io.op = 1;
    return;
  }

  if(io.op == 1){
    //encoder
    gpio_write_bit(GPIOB, 9, 1);
    uint32_t a = GPIOB->regs->IDR;
    gpio_write_bit(GPIOB, 9, 0);
    io.turns_state = (io.turns_state<<2) | ((a&0x800)>>11) | ((a&0x400)>>9);
    if((io.turns_state&0xf) == 0b1011) io.turns_right++;
    if((io.turns_state&0xf) == 0b0111) io.turns_left++;

    io.op = 0;
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

  //disableDebugPorts();
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

void lcd_fade(int in){
  gpio_set_mode(GPIOA, LCD_LIGHT_PWM, GPIO_OUTPUT_PP); 
  gpio_write_bit(GPIOA, LCD_LIGHT_PWM, 0);
  for(int i=500; i<1000; i++){ 
    delay_us(1000-i);
    gpio_write_bit(GPIOA, LCD_LIGHT_PWM, in);
    delay_us(i);
    gpio_write_bit(GPIOA, LCD_LIGHT_PWM, !in);
  }
  gpio_write_bit(GPIOA, LCD_LIGHT_PWM, 1);
}

void lcd_init(){
  if(lcd.inited) return;
  lcd.inited = 1;
  lcd.transferring = 1;

  spi_init(SPI1);
  spi_master_enable(SPI1, 
    SPI_BAUD_PCLK_DIV_8, 
    SPI_MODE_0, 
    SPI_FRAME_MSB | SPI_DFF_8_BIT | SPI_SW_SLAVE | SPI_SOFT_SS 
  );

  gpio_set_mode(GPIOA, SPI_MOSI, GPIO_AF_OUTPUT_PP); //LCD_MOSI
  gpio_set_mode(GPIOA, SPI_CLOCK, GPIO_AF_OUTPUT_PP); //LCD_CLOCK
  gpio_set_mode(GPIOA, SPI_CS, GPIO_OUTPUT_PP); //LCD_CS
  gpio_set_mode(GPIOA, SPI_RST, GPIO_OUTPUT_PP); //LCD_RST
  gpio_set_mode(GPIOA, SPI_DC, GPIO_OUTPUT_PP); //LCD_DC

  gpio_write_bit(GPIOA, SPI_CS, 1);
  gpio_write_bit(GPIOA, SPI_RST, 1);
  delay_us(100000);
  gpio_write_bit(GPIOA, SPI_RST, 0);
  delay_us(100000);
  gpio_write_bit(GPIOA, SPI_RST, 1);
  delay_us(1000000);

  //init
  gpio_write_bit(GPIOA, SPI_DC, 0);
  gpio_write_bit(GPIOA, SPI_CS, 0);
  
  spi_tx(SPI1, lcd.init_seq, lcd.init_seq_len);
  while(spi_is_busy(SPI1));
  
  gpio_write_bit(GPIOA, SPI_CS, 1);
  
  lcd.transferring = 0;
}

void lcd_update(){
  lcd_updateSection(0,8,0,128);
}

void lcd_updateSection(int pageoffset, int pagelen, int offset, int len){
  if(!spi_is_enabled(SPI1)) return;
  lcd.transferring = 1;
  for(int p=pageoffset; p<pageoffset+pagelen; p++){ 
    //page - col addy
    gpio_write_bit(GPIOA, SPI_DC, 0);
    gpio_write_bit(GPIOA, SPI_CS, 0);
    lcd.data_seq[0] = 0b10110000 | p;
    lcd.data_seq[1] = 0x4 | (0xf&offset);
    lcd.data_seq[2] = 0x10 | ((0xf0&offset)>>4);
    spi_tx(SPI1, lcd.data_seq, 3);
    while(spi_is_busy(SPI1));
    gpio_write_bit(GPIOA, SPI_CS, 1);

    int pp = (p%4)*8;
    uint32_t* buf = p < 4 ? lcd.fbuf_top : lcd.fbuf_bot;

    gpio_write_bit(GPIOA, SPI_DC, 1);
    gpio_write_bit(GPIOA, SPI_CS, 0);
    for(int d=offset; d<offset+len; d++){
      //SPI.transfer(0b01010101 << (d%2)); //checkered
      uint32_t dd = buf[d];
      dd &= (0xff<<pp);
      dd >>= pp;
      spi_tx(SPI1, &dd, 1);
      while(spi_is_busy(SPI1));
    }
    gpio_write_bit(GPIOA, SPI_CS, 1);
    delay_us(1);
  }
  lcd.transferring = 0;
}

void lcd_clearSection(int yoff, int ylen, int xoff, int xlen, int fill){
  if(yoff > 32){
    yoff -= 32;
    int bb = ((1<<ylen)-1);
    for(int i=xoff; i<xoff+xlen; i++){
      if(fill)
      lcd.fbuf_bot[i] |= (bb<<(yoff));
      else
      lcd.fbuf_bot[i] &= (~(bb<<(yoff)));
    }
  }
  else{
    int yy = yoff+ylen;
    int bt = ((1<<ylen)-1);
    int bb = yy > 32 ? ((1<<(yy-32))-1) : 0;
    for(int i=xoff; i<xoff+xlen; i++){
      if(fill){
        lcd.fbuf_top[i] |= (bt<<yoff);
        lcd.fbuf_bot[i] |= bb;
      }
      else{
        lcd.fbuf_top[i] &= (~(bt<<yoff));
        lcd.fbuf_bot[i] &= (~bb);
      }
    }
  }
  
}
void lcd_clear(){
  for(int i=0; i<128; i++){
    lcd.fbuf_top[i] = 0;
    lcd.fbuf_bot[i] = 0;
  }
}
void lcd_drawHline(int x, int y, int w){
  if(y<0) return;
  if(y>63) return;
  for(int i=x; i<x+w; i++){
    if(i<0) continue;
    if(i>127) continue;

    if(y<32)
      lcd.fbuf_top[i] |= (1<<y);
    else
      lcd.fbuf_bot[i] |= (1<<(y-32));
  }
}
void lcd_drawVline(int x, int y, int h){  
  if(x<0) return;
  if(x>127) return;
  int e = y+h > 64 ? 64 : y+h;
  for(int i=y; i<e; i++){
    if(i<0) continue;
    if(i>63) continue;

    if(i<32)
      lcd.fbuf_top[x] |= (1<<(i));
    else
      lcd.fbuf_bot[x] |= (1<<(i-32));
  }
}
void lcd_drawLine(int x, int y, int x2, int y2){
  const int dy = (y2-y);
  const int dx = (x2-x);
  const int nx = dx < 0 ? dx*-1 : dx;
  const int ny = dy < 0 ? dy*-1 : dy;

  if(dx == 0)
    return lcd_drawVline(x,y,dy);

  if(dy == 0)
    return lcd_drawHline(x,y,dx);

  if(nx > ny){
    if(dx < 0){
      for(int i=0; i>dx; i--){
          int yy = ((i)*dy)/dx;
          lcd_drawHline((i+x),yy+y,1);
      }
    }
    else{
      for(int i=0; i<dx; i++){
          int yy = ((i)*dy)/dx;
          lcd_drawHline((i+x),yy+y,1);
      }
    }
    
  }
  else{
    if(dy < 0){
      for(int i=0; i>dy; i--){
          int xx = ((i)*dx)/dy;
          lcd_drawHline(xx+x,(i+y),1);
      }
    }
    else{
      for(int i=0; i<dy; i++){
          int xx = ((i)*dx)/dy;
          lcd_drawHline(xx+x,(i+y),1);
      }
    }
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

void lcd_fillRectSize(int x, int y, int w, int h, int pattern){
  switch (pattern)
  {
  case 0: //solid
    lcd_clearSection(y,h,x,w,1);
    break;
  }
}


int lcd_drawString(int x, int y, font_t f ,const char* str){
	int ii = 0;
  int ww = 0;
	while(str[ii] != 0){
		ww += lcd_drawChar(x + ii*f.wide, y, f, str[ii++]);
	}
  return ww;
}

int lcd_drawChar(int x, int y, font_t f, char ch){
  if(ch < ' ' || ch > 126){
    ch = 0;
  }
  else{
    ch -= ' ' - 1;
  }
  
  uint8_t hh = f.tall;
  uint8_t ww = f.wide;
  
  uint32_t char_byte = 0;
  uint32_t lo_byte = 0;
  uint32_t hi_byte = 0;
  
  int g = ch*ww;
  int hl = 32-hh;
  if(y > hl && y < 32){
    //over boundary of two bufs
    for(int i=0; i<ww; i++){
      char_byte = f.tall <= 8 ? ((uint8_t*)f.data)[i + g] : ((uint16_t*)f.data)[i + g];
      lcd.fbuf_top[x  ] |= (char_byte << y);
      lcd.fbuf_bot[x++] |= (char_byte >> (hh-(y-hl)));
      if(x==128) return i;
    }
  }
  else if(y>=32){
    y -= 32;
    for(int i=0; i<ww; i++){
      lcd.fbuf_bot[x++] |= f.tall <= 8 ? ((uint8_t*)f.data)[i + g]<<y : ((uint16_t*)f.data)[i + g]<<y;
      if(x==128) return i;
    }
  }
  else{
    for(int i=0; i<ww; i++){
      lcd.fbuf_top[x++] |= f.tall <= 8 ? ((uint8_t*)f.data)[i + g]<<y : ((uint16_t*)f.data)[i + g]<<y;
      if(x==128) return i;
    }
  }

  return ww;

}