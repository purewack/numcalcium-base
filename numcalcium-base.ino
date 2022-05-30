#include "hw.h"

void setup(){
    Serial.begin(9600);
    base_init();
    lcd_fade(1);
    lcd_clear();

    lcd_drawRectSize(0,0,40,40);
    lcd_drawLine(20,20,40,30);
    lcd_drawLine(20,20,22,40);
    lcd_drawLine(20,20,40,40);
    lcd_drawCharSmall(0,48,1);
    lcd_drawCharSmall(16,48,'1');
    lcd_drawCharSmall(24,48,'2');
    lcd_drawCharSmall(32,48,'3');

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
    // lcd_drawLine(5,15,5+d*2,15+d);
    // lcd_update();

    // delay(100);
}
