# API for NumCalcium projects

# Software mappings of Hardware
<i>See: SW_MAP.md</i>

## LCD

```
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

#define DRAWBITMAP_SOLID 0
#define DRAWBITMAP_TRANS 1
#define DRAWBITMAP_XOR 2
int lcd_drawTile(int x, int y, int w, int h, int sbuf, int buflen, void* buf, int mode);
int lcd_drawTile(int x, int y, int w, int h, int sbuf, void* buf, int mode);
```

## Buttons and Encoder

```
void base_init();
void base_deinit();

void io_mux_init();
void io_mux_irq();

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
  int turns_left;
  int turns_right;
  int turns_state;
  const uint8_t seq_row[6] = {14,15,3,4,8,9};
  uint8_t row;
  uint8_t op;
  uint8_t inited = 0;
};
```

## ADC 
```
extern uint32_t adc_state;
extern uint32_t adc_srate;
#define IS_ADC_BUSY (DMA1->regs->CNDTR1)
void adc_set_srate_type(int8_t type);
void adc_block_get(uint16_t* buf, uint16_t n);
void adc_block_get_complex(uint32_t* buf, uint16_t n);
void adc_block_init();
void adc_block_deinit();

```