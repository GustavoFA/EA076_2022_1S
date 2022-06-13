#include <Wire.h>

#define ADD 0x50    // 0101 0000

byte vetor[2];
byte t = 0x00;
char num = 33;

void setup() {

  Serial.begin(9600);

  Wire.begin();

  pinMode(13, OUTPUT);
  digitalWrite(13, 0);

  delay(1000);
  
  //Write(0x05, 64);
  //Read(0x05);

}


void loop() {
	//_delay_ms(1);
  
  
  if(t<0xFF) {
    Write(t, num);
  	Read(t);
    num++;
    t++;
  }
  
  /*Write(0x05, 65);
  Write(0x06, 66);
  _delay_ms(1000);
  Read(0x05);
  Read(0x06);
  */
  _delay_ms(1000);
}

void Write(byte Add, byte Data){
  vetor[0] = Add;
  vetor[1] = Data;
  Serial.println("Entrou aqui Write!");
  Wire.beginTransmission(ADD);
  Wire.write(vetor,2);
  //Wire.write(Data);
  Wire.endTransmission();

  _delay_ms(600); // tempo de espera para escrita + 100ms
  
}

void Read(byte Add){
  Serial.println("Entrou aqui Read!");
  char DATA;

  Wire.beginTransmission(ADD);
  Wire.write(Add);
  Wire.endTransmission();
  Wire.requestFrom(ADD, 1);
  delay(1);
  if(Wire.available()){
    DATA = Wire.read(); 
    Serial.println(DATA);
  }
}
