#include "hw.h"

void setup(){
    Serial.begin(9600);
    base_init();
    lcd_fade(1);
}

int d = 0;
void loop(){
    if(io.bscan_down){
        Serial.println(io.bscan_down);
        io.bscan_down = 0;
    }

    d = (d+1)%40;
    lcd_clear();
    lcd_drawRectSize(5,5,d,d);
    lcd_update();

    delay(20);
}
