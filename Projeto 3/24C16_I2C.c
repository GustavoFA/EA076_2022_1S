// C++ code
//

#include <Wire.h>

byte dado = 0;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
}

void loop()
{
  
  // Escrever 32 na posição 0b10100000 --> endereço + escrita
  // 0b0010 0000 --> dado
  
  
  Wire.beginTransmission(0x50);
  Wire.write(0x01);
  Wire.write(76);
  Wire.endTransmission();
  
  _delay_ms(10);

  // Ler --> 0b10100001 --> endereço + leitura
  
  Wire.beginTransmission(0x50);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.beginTransmission(0xA1);
  dado = Wire.read();
  Wire.endTransmission();
  
  Serial.println(dado);
  
  _delay_ms(500);

}
