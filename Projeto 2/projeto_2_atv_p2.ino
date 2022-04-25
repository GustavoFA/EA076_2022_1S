// Atividade Parcial 2 - Estimativa da frequência de rotação do eixo de um motor a partir de encoder
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

/* A saída do encoder é conectado no pino A0, que corresponde ao pino da interrupção PCINT8 
   (Interrupção Pin Change) */


long int n = 0; // variavel de contagem de variacao de nivel logico
volatile unsigned char cont = 0; // variavel de contagem do temporizador
unsigned int rpm = 0;

/* rotina de servico de interrupcao que eh executada toda vez ha mudanca de 
estado e que mede a quantidade de vezes que essa interrupcao foi chamada */
ISR (PCINT1_vect) {
    /* cada vez que a interrupcao eh chamada, o valor de n eh incrementado
       mas levando em conta que o PIN CHANGE eh executado quando ha variacao de 
       nivel logico, isto eh, quando for calculado a frequencia estimada, 
       o valor de n deve ser dividido por 2 */
    n++;
}

void setup(){

    cli(); // desabilita as interrupcoes
    configuracao_Timer0(); // Configura o temporizador
    PCICR |= 0x02; // Habilita as interrupcoes para o PCINT8 (A0)
    PCMSK1 |= 0x01; // Habilita o disparo das interrupcoes
    sei(); // Habilita as interrupcoes

    pinMode(A0, INPUT); // Pino A0 como entrada

    Serial.begin(9600); // Inicializo o Serial
 
}

void frequencia(void) {

    // Passado o 1s, é calculado a estimativa da frequência de rotação
    if (cont>=125) {
        rpm = 60*((n/2))/(60);
        Serial.print(rpm);
        Serial.println();
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
}
