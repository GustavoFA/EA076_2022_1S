// Atividade Parcial 5 - Acionamento do display via protocolo I2C
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867



#include <Wire.h>

unsigned int rpm = 3210; // Variavel utilizada para o calculo da frequência de rotação
unsigned int contDisplay = 0;      // Variavel de contagem utilizado para a multiplexacao dos displays de pedestre e de carros, fica intercalando entre um estado e outro a cada 16ms
// algarismos de unidade, dezena, centena e milhar
int u;
int d;
int c;
int m;
bool troca = 1; // variável auxiliar para comunição I2C ocorrer a cada interrupção do temporizador




void setup(){

    configuracao_Timer0(); // Configura o temporizador Timer 0

    pinMode(A0, INPUT); // Pino A0 como entrada
    
    Serial.begin(9600); // Serial para Debug
    
    Wire.begin();       // Inicio o Wire

    // Toda vez que houver a obtenção de um valor novo do encoder deverá ser feito a seraração das casas decimais e armazenadas como mostrado a seguir:
    m =  rpm / 1000;
    c = (rpm - 1000*m) / 100;
    d = (rpm - 1000*m - 100*c) / 10;
    u = (rpm - 1000*m - 100*c - 10*d);
    
}


void loop(){

    // Mudança do display e seu valor só ocorrerá a cada interrupção do temporizador, no caso 4 ms, isso está relacionado à variável troca
    if(troca) visor(u, d, c, m);

}

// Função que obtém valor RPM do motor e apresenta no LCD utilizando a interface I2C (TWI)
void visor(int x, int y, int w, int z ){
    
    switch (contDisplay)
    {
    case 0:
        Wire.beginTransmission(32);
        Wire.write(112 + x);  // Display da unidade ligado + valor da unidade
        Wire.endTransmission();
        break;
    case 1:
        Wire.beginTransmission(32);
        Wire.write(176 + y);  // Display da dezena ligado + valor da dezena
        Wire.endTransmission(); 
        break;
    case 2:
        Wire.beginTransmission(32);
        Wire.write(208 + w);  // Display da centena ligado + valor da centena
        Wire.endTransmission();
        break;
    case 3:
        Wire.beginTransmission(32);
        Wire.write(224 + z);  // Display do milhar ligado + valor do milhar
        Wire.endTransmission();
        break;
    }

    // Indico que a comunicação I2C já ocorreu
    troca = 0;

    // Incremento para mudar alternar entre qual display ficará aceso
    contDisplay++;

    // Quando ultrapassar o valor do último display reseto a posição 
    if(contDisplay > 3) contDisplay = 0;

}


void configuracao_Timer0(){
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* Configuracao Temporizador 0 (8 bits) para gerar interrupcoes periodicas a cada 8ms no modo Clear Timer on Compare Match (CTC)*/
    // Relogio = 16e6 Hz
    // Prescaler = 256
    // Faixa = 249 (contagem de 0 a OCR0A = 249)
    // Intervalo entre interrupcoes: (Prescaler/Relogio)*Faixa = (256/16e6)*(249+1) = 4 ms

    // TCCR0A – Timer/Counter Control Register A
    // COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
    // 0      0      0      0          1     0
    TCCR0A = 0x02;

    // OCR0A – Output Compare Register A
    OCR0A = 249;

    // TIMSK0 – Timer/Counter Interrupt Mask Register
    // – – – – – OCIE0B OCIE0A TOIE0
    // – – – – – 0      1      0
    TIMSK0 = 0x02;

    // TCCR0B – Timer/Counter Control Register B
    // FOC0A FOC0B – – WGM02 CS02 CS01 CS0
    // 0     0         0     1    0    0
    TCCR0B = 0x04;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

// Rotina de servico de interrupcao do temporizador 
ISR(TIMER0_COMPA_vect){

    // Indico que passou o tempo do temporizador e que podemos enviar um novo dado via I2C e trocar o display
    troca = 1;

}
