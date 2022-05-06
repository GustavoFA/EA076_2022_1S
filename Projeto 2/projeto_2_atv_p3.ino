// LCD 
// Referência - https://www.filipeflop.com/blog/como-utilizar-o-display-lcd-16x2/

#include <LiquidCrystal.h>

// Pretende usar o PD5(~), PD4, PD3(~) e PD2
// DB4 DB5 DB6 DB7
// Vss - GND
// Vdd - Vcc
// V0 - Sinal Analogico
// Rs - PB4 (12)
// Rw - GND
// E  - PB3 (11)
// A  - Vcc
// K  - GND

LiquidCrystal lcd(12, 11, 5, 7, 6, 8);

String rpm;

void setup(){

    lcd.begin(16,2);

}


void print_lcd(String RPM){

    lcd.clear();

    lcd.setCursor(0, 0);

    lcd.print("Frequencia:");
    lcd.setCursor(3, 1);

    lcd.print(RPM);
    
    lcd.setCursor(7, 1);
    lcd.print("rpm");

}


void loop(){


    print_lcd("123");
    delay(2000);
    print_lcd("456");
    delay(2000);

    

}
