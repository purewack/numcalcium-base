#include "hw.h"

void setup(){
    Serial.begin(9600);
    base_init();
    io_mux_init();
    io.lcd_fade = 5;
    lcd_init();
    lcd_clear();
    lcd_update();
    lcd_drawVline(0,0,16);
    lcd_drawHline(0,0,16);
    lcd_drawVline(5,16,16);
    lcd_update();
    Serial.println("done");
}

int d = 0;
void loop(){
    if(io.bscan_down){
        if(io.bscan_down & BUTTON(K_F1)) io.lcd_fade = 3;
        if(io.bscan_down & BUTTON(K_F2)) io.lcd_fade = -3;
        Serial.println(io.bscan_down);
        io.bscan_down = 0;
    }

//     d = (d+1)%40;
//     lcd_clear();
//     lcd_drawRect(5,5,d,d);
//     lcd_update();
}
