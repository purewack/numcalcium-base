#include "hw.h"

void setup(){
    Serial.begin(9600);
    base_init();
    lcd_fade(1);
    lcd_clear();

    lcd_drawLine(0,0,15,13);
    lcd_drawCharSmall(16,23,0);
    lcd_drawCharSmall(32,24,0);
    lcd_drawCharSmall(48,31,0);
    lcd_drawCharSmall(64,32,0);

    lcd_update();
}

int d = 0;
void loop(){
    if(io.bscan_down){
        Serial.println(io.bscan_down);
        io.bscan_down = 0;
    }

    // d = (d+1)%40;
    // lcd_clear();
    // lcd_drawRectSize(5,5,d,d);
    // lcd_update();

    delay(20);
}
