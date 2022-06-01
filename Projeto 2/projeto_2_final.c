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
#include <LiquidCrystal.h>

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

// ---- Display 16x2 ----
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

LiquidCrystal lcd(12, 11, 5, 7, 6, 8); // Definicao dos pinos que serao utilizados para a ligacao do display

// ---------------------------

// ------ Comandos -----------
// Buffer para armazernar temporariamente os bytes lidos na serial  
byte bufferByte[8];     // maior entrada terá 8 bytes contando com o *

// Contador para identificar e manipular elementos no vetor buffer
int cont_vet = 0;

// Vetor para guardar mensagem recebida e futuramente ser entregue para o identificador
char mensagem[7];       // maior mensagem a ser armazenada terá 7 bytes

// Variável para armazenar tamanho da mensagem recebida
int tam_msg = 0;


String vel = ""; // String que armazena o valor da velocidade do comando "VEL"

String codigo = "";

// --------------------------------

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
    PCICR |= 0x02; // Habilita as interrupcoes para o PCINT8 (A0)
    PCMSK1 |= 0x01; // Habilita o disparo das interrupcoes
    sei(); // Habilita as interrupcoes

    configuracao_Timer0(); // Configura o temporizador Timer 0 -> 4 ms

    pinMode(A0, INPUT); // Pino A0 como entrada -> Encoder
    
    Serial.begin(9600); // Serial para Debug
    
    Wire.begin();       // Inicio o Wire

    /* Definicao do numero de linhas e colunas do LCD */
  	lcd.begin(16,2);
  	//Inicio o LCD com ele apagado
 	lcd.clear();
    
}


void loop(){

    // Obtenção da frequência
    frequencia();

    // Mudança do display e seu valor só ocorrerá a cada interrupção do temporizador, no caso 4 ms, isso está relacionado à variável troca
    if(troca) visor(u, d, c, m);

    fun_deco();

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

// Função que retornar se a mensagem já foi recebida
bool fun_receber(){
    
    // Verifico quantidade de bytes disponíveis para leitura na serial
    while(Serial.available() > 0) {
    
    // Armazeno em um buffer
    bufferByte[cont_vet] = Serial.read();

    // Verifico se o caracter lido é * (ASCII = 42)
    if(bufferByte[cont_vet] == 42){

      // Se for guardo todos os caracteres em um vetor e guardo seu tamanho
      tam_msg = cont_vet;
      for(int i = 0; i < tam_msg; i++){
      	mensagem[i] = bufferByte[i];
      }

      // Retorno que a mensagem foi lida e zero o contador para futuras leituras
      cont_vet = 0;
      return 1;

    }else{

      // Avanço o contador
      cont_vet++;
        
      // Reseto a posição de indicação do elemento do vetor -> Faço isso, pois, se caso algum usuário inserir 8 bytes sem nenhum *
      // e como não há nenhum comando com mais de 8 bytes
      if(cont_vet > 7) cont_vet = 0;

      // Retorno que a mensagem ainda não foi recebida (precisa do * para saber identificar que uma mensagem chegou)
      return 0;
    }
  }
    
}

// Função que decodifica a mensagem que foi enviada ao monitor, e para o caso de setar a velocidade, retorna o valor da velocidade
long fun_deco() {
    
    // Verifica se a variavel de sinalizacao de mensagem foi setada
    if(fun_receber()) {
        
        lcd.setCursor(0,0) // Cursor na coluna 0 e linha 0
      	String codigo = ""; 
      	// Armazena os elementos do buffer dentro da variavel do tipo String para manipulacoes futuras
        for(int j=0; j<tam_msg;j++){
          codigo.concat(mensagem[j]);
        }
        
        // Verifica qual comando foi escrito no monitor serial, para enviar a UART sua respectiva mensagem (de erro ou nao)
        if (codigo.equalsIgnoreCase("VENT")) {
            
            lcd.print("OK VENT"); 
        }
        else if (codigo.equalsIgnoreCase("EXAUST")) {
            
            lcd.print("OK EXAUST"); 
        }
        else if (codigo.equalsIgnoreCase("PARA")) {
            
            lcd.print("OK PARA"); 
        }
        else if (codigo.equalsIgnoreCase("RETVEL")) {
            
            lcd.print("VEL: ");
            lcd.setCursor(4,1);
            lcd.print(rpm);    // coloca o valor da velocidade no instante
            lcd.setCursor(8,1);
        }
        
        /* Comando de velocidade - para esse comando, é feito a identificacao do comando "VEL" atraves dos 3 primeiros elementos da variavel 'codigo'
           e, após isso, é verificado se a o numero que seta a velocidade do motor esta no formato xxx (xxx entre 000 e 100) */
        else if (codigo.substring(0, 3).equalsIgnoreCase("VEL")) {

            codigo.remove(0,4); // Remove os 4 primeiros elementos do comando ('VEL '), para que seja trabalhado somente com os numeros
			
            /* Verifica se o numero do comando enviado está no formato correto ('xxx'), e para isso, utiliza-se o length() para ver se o tamanho do comando
               enviado corresponde com o tamanho do formato desejado */
            if (codigo.length() == 3) {
                
                vel = codigo; // Guarda os numeros em uma outra variavel STRING, que sera manipulada
                
                /* Verifica se o codigo enviado é realmente um numero */
                if (isdigit(vel[0]) && isdigit(vel[1]) && isdigit(vel[2])) {
                    
                    /* Verifica se o numero esta dentro do limite de 000 a 100 e retorna o seu valor inteiro */
                    if (vel.toInt() >= 0 && vel.toInt() <= 100) {
                        
                        lcd.print("OK VEL ");
                        lcd.setCursor(7,1);
                        lcd.print(vel);
                        lcd.setCursor(12,1);
                        lcd.print("%");
                        
                        return vel.toInt();
                    }
                    /* Caso contrario, envia a mensagem de parametro incorreto */
                    else {
                        
                        lcd.print("ERRO: PARAMETRO INCORRETO");
                    }
                }
                /* Caso contrario, envia a mensagem de parametro incorreto*/
                else {
                    
                    lcd.print("ERRO: PARAMETRO INCORRETO");
                }
            }
            /* Caso o tamanho do comando enviado não corresponda ao tamanho desejado, é enviado a mensagem de parametro incorreto */
            else if (codigo.length() < 3 && codigo.length() > 0) {
                
                lcd.print("ERRO: PARAMETRO INCORRETO");
            }
            /* Caso o tamanho do numeros for 0, isto é, não foi dgitado nenhum numero, é enviado uma mensagem de parâmetro ausente*/
            else {
                
                lcd.print("ERRO: PARAMETRO AUSENTE");
            }
        }

        /* Caso a mensagem enviada ao monitor serial não corresponda com nenhum dos casos anteriores ou que a mensagem extrapolou o tamanho 
           definido (tamanho maximo do buffer é de 8 - contando com o *) é enviado a mensagem de erro de comando inexistente */ 
        else {
            
            lcd.print("ERRO: COMANDO INEXISTENTE")
        }
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
