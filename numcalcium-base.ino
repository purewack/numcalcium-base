#include "hw.h"

void setup(){
    Serial.begin(9600);
    base_init();
    io_mux_init();
    io.lcd_fade = 5;
}

void loop(){
    if(io.bscan_down & BUTTON(5)){
        Serial.println("down 5");
        C_BIT(io.bscan_down, 5);
    }
}
