// Atividade Parcial 1 - Leitura e escrita na memória EEPROM 24C16
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

#include <Wire.h>

#define ADD 0x50    // 0101 0000
#define WP 13       // Write Protect

// Vetor para endereço + dado
byte vetor[2];

// Endereço
byte t = 0x00;

// Dado
char num = 33;

void setup() {

  Serial.begin(9600);

  Wire.begin();

  // GPIO para Write Protect
  pinMode(WP, OUTPUT);
  digitalWrite(WP, 0);

  delay(1000);

}


void loop() {
  
  // Escrita incrementada para teste
  if(t<0xFF) {
    Write(t, num);
  	Read(t);
    num++;
    t++;
  }
  
  _delay_ms(1000);
}

// Função de escrita
void Write(byte Add, byte Data){
  vetor[0] = Add;   // Byte de endereço
  vetor[1] = Data;  // Byte de dado
  Wire.beginTransmission(ADD);
  Wire.write(vetor,2);
  Wire.endTransmission();

  _delay_ms(600); // tempo de espera para escrita + 100ms
  
}

// Função de leitura
void Read(byte Add){

  char DATA;

  Wire.beginTransmission(ADD);
  Wire.write(Add);
  Wire.endTransmission();
  Wire.requestFrom(ADD, 1);
  _delay_ms(1);
  if(Wire.available()){
    DATA = Wire.read(); 
    Serial.println(DATA);
  }
}
