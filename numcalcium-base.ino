#include "hw.h"
#include <USBComposite.h>
USBHID dd;
HIDKeyboard Key(dd);
USBMIDI USB_midi;

void setup(){
    Serial.begin(9600);
    base_init();
    lcd_fade(1);
    lcd_clear();

    lcd_drawRectSize(0,0,40,40);
    lcd_drawLine(20,20,40,30);
    lcd_drawLine(20,20,22,40);
    lcd_drawLine(20,20,40,40);
    lcd_drawStringTiny(16,48,"Hello World !");

    lcd_update();
}

int d = 0;
char str[20];
void loop(){
    if(io.bscan_down){
        Serial.println(io.bscan_down);
        io.bscan_down = 0;
    }

    sprintf(str,"Count %d", d);

    d++;
    const int xx = 32-4;
    lcd_clearSection(xx,8,4*6,10*6);
    lcd_drawStringTiny(0,xx,str);
    lcd_updateSection(3,2,4*6,10*6);

    // delay(100);
}
