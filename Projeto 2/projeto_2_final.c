// Projeto 2 - CONTROLADOR DE VENTILADOR DE TETO
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867


// Módulos:
/*

    * Visor -> Tem a função, é só passar os algarismos para ela representar
    * Motor -> Tem a função. Lembrando que ela deve ser chamada pela função de recebimento/decodificação do comando
    * Encoder -> Tem a função, que estima a frequência e separa os algarismos

*/

// Usamos Timer 0 -> 4 ms


#include <Wire.h>

// ----- Visor 7 segmentos -----
unsigned int contDisplay = 0;      // Variavel de contagem utilizado para a multiplexacao dos displays de pedestre e de carros, fica intercalando entre um estado e outro a cada 16ms

// algarismos de unidade, dezena, centena e milhar
int u = 0;
int d = 0;
int c = 0;
int m = 0;

bool troca = 1; // variável auxiliar para comunição I2C ocorrer a cada interrupção do temporizador
// -------------------------------


// ---- Motor + Encoder -----
long int n = 0; // variavel de contagem de variacao de nivel logico
volatile unsigned char cont = 0; // variavel de contagem do temporizador
unsigned int rpm = 0; // Variavel utilizada para o calculo da frequência de rotação
// --------------------------


void setup(){

    cli(); // desabilita as interrupcoes
    PCICR |= 0x02; // Habilita as interrupcoes para o PCINT8 (A0)
    PCMSK1 |= 0x01; // Habilita o disparo das interrupcoes
    sei(); // Habilita as interrupcoes

    configuracao_Timer0(); // Configura o temporizador Timer 0 -> 4 ms

    pinMode(A0, INPUT); // Pino A0 como entrada -> Encoder
    
    Serial.begin(9600); // Serial para Debug
    
    Wire.begin();       // Inicio o Wire
    
}


void loop(){

    // Obtenção da frequência
    frequencia();

    // Mudança do display e seu valor só ocorrerá a cada interrupção do temporizador, no caso 4 ms, isso está relacionado à variável troca
    if(troca) visor(u, d, c, m);

}


// Função para inserir tipo de movimento e tensão média (PWM)
void motor(char mov, int PWM){

    switch (mov)
    {
    // Paro o movimento dos motores
    PORTD &= ~0x14;
    case 'p':
        // Mantenho parado
        break;
    
    case 'a':
        // Movimento horário
        PORTD |= 0x04;
        break;

    case 'h':
        // Movimento anti-horário
        PORTD |= 0x10;
        break;
    }

    // Faixa de comparação: 0 --> 255
    // PWM: 0 --> 100 %
    // 1% = 25.5 
    OCR2B = (int) PWM*(2.55);

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

// Função para cálculo da frequência e separação dos algarismos
void frequencia(void) {

    // Passado o 1s, é calculado a estimativa da frequência de rotação
    if (cont>=250) {

      /* Para calcular a frequência de rotação do motor, utiliza-se a seguinte fórmula, 
         f_rpm = 60*(numero de pulsos/numero de pulsos por volta), onde o numero de pulsos é dado por n/2 (já que 
         o n é incrementado a cada variação de nível lógico), o numero de pulsos por volta é dado pelo encoder utilizado,
         e o 60 é utilizado para converter de Hz para rpm */
      rpm = 60*((n/2))/(60);  // Para o simulador
      // rpm = 60*((n/2))/(2);   // Para o circuito original       

      // Reseto variáveis de contagem
      n = 0;
      cont = 0;
      
      // Armazeno os algarismos da medida de frequência
      m =  rpm / 1000;
      c = (rpm - 1000*m) / 100;
      d = (rpm - 1000*m - 100*c) / 10;
      u = (rpm - 1000*m - 100*c - 10*d);

    }
}

// Temporizador de 4 ms
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

// Rotina de servico de interrupcao do temporizador de 4 ms
ISR(TIMER0_COMPA_vect){

    cont++;

    // Indico que passou o tempo do temporizador e que podemos enviar um novo dado via I2C e trocar o display
    troca = 1;

}
