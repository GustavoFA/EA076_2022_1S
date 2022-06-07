// Atividade Parcial 1 - Recepção e decodificação de comandos
// Gustavo Freitas Alves & Jitesh Ashok Manilal Vassaram


// -------------------------------------------------------------------------------

#include <Wire.h>

//byte dado = 0;
//unsigned char registrador_de_endereco = 0;
//unsigned char memoria[256];
unsigned char palavra;

void setup()
{
    Wire.begin(0x50);         
    Wire.onReceive(escrever);
    Wire.onRequest(ler);
    Serial.begin(9600);
}

void loop()
{
  
   _delay_ms(1);

   escrever(0, 76);
   _delay_ms(500);
   ler(0);
   Serial.println(palavra);

}



void escrever(unsigned int add, unsigned char dado) {

    Wire.beginTransmission(0x50);
    Wire.write(add);
    Wire.write(dado);
    Wire.endTransmission();
}

unsigned char ler(unsigned int add) {

    while (Wire.available())
    {
        Wire.beginTransmission(0x50);
        Wire.write(add);
        palavra = Wire.read();
        Wire.endTransmission();
    }
    return palavra
}
