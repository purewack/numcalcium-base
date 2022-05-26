#include "hw.h"

void setup(){
    Serial.begin(9600);
    base_init();
    io_mux_init();
    io.lcd_fade = 5;
}

void loop(){
    if(io.bscan_down){
        if(io.bscan_down & BUTTON(K_F1)) io.lcd_fade = 3;
        if(io.bscan_down & BUTTON(K_F2)) io.lcd_fade = -3;
        Serial.println(io.bscan_down);
        io.bscan_down = 0;
    }
}
