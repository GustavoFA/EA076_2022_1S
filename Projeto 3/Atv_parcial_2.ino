// Atividade Parcial 2 - Sensor de Temperatura LM35
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867


/*

    - Usar referência interna de 1,1 V

    - Resolução = (Vmax - Vmin)/(2^n)  --> n = número de bits (Arduino UNO = 10)

    - Resolução será de 0,1 °C/LSB

    - 10mV/°C

*/

#define Temp 0

void setup(){

    Serial.begin(9600);

    pinMode(0, INPUT);
    analogReference(INTERNAL);

}


void loop(){

    _delay_ms(1000);

    byte dado;

    dado = analogRead(Temp);

    Serial.println(dado*0.1, DEC);


}
