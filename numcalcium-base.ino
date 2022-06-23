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
    font_t fonttiny = {fonttiny_tall,fonttiny_wide, (void*)fonttiny_data, fonttiny_data_count};
    lcd_drawString(16,48, fonttiny, "Hello World !");
    lcd_fillRectSize(65,5,30,10, 0);

    lcd_update();
}

double th = 0.0;
int d = 0;
char str[20];
void loop(){
    // if(io.bscan_down){
    //     Serial.println(io.bscan_down);
    //     io.bscan_down = 0;
    // }

    // sprintf(str,"Count %d", d);

    // d++;
    // const int xx = 32-4;
    // lcd_clearSection(xx,8,4*6,10*6,0);
    // lcd_drawStringTiny(0,xx,str);
    // lcd_updateSection(3,2,4*6,10*6);
    
    
    
    // for(int i=20; i<60; i++){
    //     lcd_clear();
    //     lcd_drawRectSize(20,20,40,40);
    //     lcd_drawLine(40,40,60,i);
    //     lcd_drawLine(62,20,62,60);
    //     lcd_drawHline(40,40,1);
    //     lcd_update();
    //     delay(50);
    // }
    // for(int i=20; i<60; i++){
    //     lcd_clear();
    //     lcd_drawRectSize(20,20,40,40);
    //     lcd_drawLine(40,40,80-i,60);
    //     lcd_drawLine(20,62,60,62);
    //     lcd_drawHline(40,40,1);
    //     lcd_update();
    //     delay(50);
    // }
    // for(int i=20; i<60; i++){
    //     lcd_clear();
    //     lcd_drawRectSize(20,20,40,40);
    //     lcd_drawLine(40,40,20,80-i);
    //     lcd_drawLine(18,20,18,60);
    //     lcd_drawHline(40,40,1);
    //     lcd_update();
    //     delay(50);
    // }
    // for(int i=20; i<60; i++){
    //     lcd_clear();
    //     lcd_drawRectSize(20,20,40,40);
    //     lcd_drawLine(40,40,i,20);
    //     lcd_drawLine(20,18,60,18);
    //     lcd_drawHline(40,40,1);
    //     lcd_update();
    //     delay(50);
    // }


    lcd_clearSection(32-20,40,64-20,40,0);
    lcd_drawLine(64 + int(14.0*cos(th)),32 + int(14.0*sin(th)),64 + int(14.0*cos(th+3.1415)),32 + int(14.0*sin(th+3.1415)));
    lcd_drawLine(64 + int(20.0*cos(th+2.09)),32 + int(20.0*sin(th+2.09)),64 + int(20.0*cos(th+3.1415+2.09)),32 + int(20.0*sin(th+3.1415+2.09)));
    lcd_drawLine(64 + int(6.0*cos(th+4.18)),32 + int(6.0*sin(th+4.18)),64 + int(14.0*cos(th+3.1415+4.18)),32 + int(14.0*sin(th+3.1415+4.18)));
    lcd_update();
    th += 0.007;
}
