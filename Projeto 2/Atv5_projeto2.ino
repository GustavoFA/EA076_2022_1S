// Atividade Parcial 5 - Acionamento o display via protocolo I2C
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

/* A saída do encoder é conectado no pino A0, que corresponde ao pino da interrupção PCINT8 
   (Interrupção Pin Change) */

#include <Wire.h>

long int n = 0; // variavel de contagem de variacao de nivel logico
volatile unsigned char cont = 0; // variavel de contagem do temporizador
unsigned int rpm = 0; // Variavel utilizada para o calculo da frequência de rotação
volatile unsigned int contDisplay = 0;      // Variavel de contagem utilizado para a multiplexacao dos displays de pedestre e de carros, fica intercalando entre um estado e outro a cada 16ms
unsigned char estadoMultiplex = 0;          // Variavel de estado utilizado para a mudanca de estados do multiplexador dos displays de pedestre e de carro
int m = 0; // algarismo do milhar
int c = 0; // algarismo da centena
int d = 0; // algarismo da dezena
int u = 0; // algarismo da unidade



/* rotina de servico de interrupcao que eh executada toda vez ha mudanca de 
estado e que mede a quantidade de vezes que essa interrupcao foi chamada */
ISR (PCINT1_vect) {
    /* cada vez que a interrupcao eh chamada, o valor de n eh incrementado
       mas levando em conta que o PIN CHANGE eh executado quando ha variacao de 
       nivel logico, isto eh, quando for calculado a frequencia estimada, 
       o valor de n deve ser dividido por 2 */
    n++;
}

// Definir os registradores do PIN CHANGE PCMSK e PCICR
void setup(){

    cli(); // desabilita as interrupcoes
    configuracao_Timer0(); // Configura o temporizador
    PCICR |= 0x02; // Habilita as interrupcoes para o PCINT8 (A0)
    PCMSK1 |= 0x01; // Habilita o disparo das interrupcoes
    sei(); // Habilita as interrupcoes

  	pinMode(A0, INPUT); // Pino A0 como entrada
    
    //Serial.begin(9600); // Inicializa o temporizador
    Wire.begin();
    
	
}


// Funcao de multiplexacao dos 4 displays (a cada 1/120 segundos (8,33 ms) um display fica ligado e outro desligado e fica alternando) - é utilizado uma maquina de estados de 2 estados
void multiplexDisplay() {
  
  
  if ((estadoMultiplex == 0) && (contDisplay >=2)) {
    estadoMultiplex = 1;
    
    contDisplay = 0; // Zera a contagem dessa variavel de contagem da multiplexacao
    
  }
  
  if ((estadoMultiplex == 0) && (contDisplay < 2)) {
    estadoMultiplex = 0;
  }
	
  // Estado 1 para o estado 0 - 
  if ((estadoMultiplex == 1) && (contDisplay >= 2)) {
    estadoMultiplex = 0;
    contDisplay = 0; // Zera a contagem dessa variavel de contagem da multiplexacao
    

  }
  // caso nao chegue em 16ms, permanece no mesmo estado (display carro ligado e display pedestre desligado)
  if ((estadoMultiplex == 1) && (contDisplay < 2)) {
    estadoMultiplex = 1;
  }
}

void frequencia(void) {
  
  
  // Passado o 1s, é calculado a estimativa da frequência de rotação
  if (cont>=125) {

    /* Para calcular a frequência de rotação do motor, utiliza-se a seguinte fórmula, 
        f_rpm = 60*(numero de pulsos/numero de pulsos por volta), onde o numero de pulsos é dado por n/2 (já que 
        o n é incrementado a cada variação de nível lógico), o numero de pulsos por volta é dado pelo encoder utilizado,
        e o 60 é utilizado para converter de Hz para rpm */
    rpm = 60*((n/2))/(60);

    m = rpm / 1000;
    c = (rpm - 1000*m) / 100;
    d = (rpm - 1000*m - 100*c) / 10;
    u = (rpm - 1000*m - 100*c - 10*d);


    /*Wire.beginTransmission(32);
    Wire.write(rpm, 4);
    Wire.endTransmission();*/
    

    n = 0;
    cont = 0;
  }
}

void loop(){
  
  _delay_ms(1);
  frequencia();
  
}

void configuracao_Timer0(){
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /* Configuracao Temporizador 0 (8 bits) para gerar interrupcoes periodicas a cada 8ms no modo Clear Timer on Compare Match (CTC)*/
  // Relogio = 16e6 Hz
  // Prescaler = 1024
  // Faixa = 125 (contagem de 0 a OCR0A = 124)
  // Intervalo entre interrupcoes: (Prescaler/Relogio)*Faixa = (64/16e6)*(124+1) = 0.008s

  // TCCR0A – Timer/Counter Control Register A
  // COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
  // 0      0      0      0          1     0
  TCCR0A = 0x02;

  // OCR0A – Output Compare Register A
  OCR0A = 124;

  // TIMSK0 – Timer/Counter Interrupt Mask Register
  // – – – – – OCIE0B OCIE0A TOIE0
  // – – – – – 0      1      0
  TIMSK0 = 0x02;

  // TCCR0B – Timer/Counter Control Register B
  // FOC0A FOC0B – – WGM02 CS02 CS01 CS0
  // 0     0         0     1    0    1
  TCCR0B = 0x05;
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

// Rotina de servico de interrupcao do temporizador
ISR(TIMER0_COMPA_vect){
 
  cont++; // variavel de contagem para o calculo da frequência de rotação
  contDisplay++; 
}