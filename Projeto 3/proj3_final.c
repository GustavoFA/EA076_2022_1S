// Projeto 3 - GRAVADOR DE DADOS AMBIENTAIS
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

// Usamos Timer 0 -> 4 ms


// Bibliotecas 
#include <Wire.h>
#include <LiquidCrystal.h>

#define ADD 0x50    // 0101 0000 - ENDERECO DO 24c16

// ----- Visor 7 segmentos -----
unsigned int contDisplay = 0;      // Variavel de contagem utilizado para a multiplexacao dos displays de pedestre e de carros, fica intercalando entre um estado e outro a cada 16ms

// algarismos de unidade, dezena, centena e milhar
int u = 0;
int d = 0;
int c = 0;
int m = 0;

bool troca = 1; // variável auxiliar para comunição I2C ocorrer a cada interrupção do temporizador
// -------------------------------

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

// Variável que irá bloquear o LCD de ficar frequentemente atualizando-se 
bool pode_entrar = 0;

bool pode_escrever = 0;

// Vetor utilizado para armazenar na EEPROM 
byte vetor[2];


// Variavel da temperatura
unsigned int dado;

String codigo = "";

int cod_anterior = 0;

byte LSB;
byte MSB;
char LSB_lido;
char MSB_lido;

void setup(){

    cli(); // desabilita as interrupcoes
    PCICR |= 0x02; // Habilita as interrupcoes para o PCINT8 (A0)
    PCMSK1 |= 0x01; // Habilita o disparo das interrupcoes
    sei(); // Habilita as interrupcoes

    configuracao_Timer0(); // Configura o temporizador Timer 0 -> 4 ms

    // Configuração das GPIOs PD
    DDRD |= 0x1C;   // Saídas PD2, PD3 e PD4
    PORTD &= ~0x14; // Início em nível baixo para deixar o motor parado

    // Configuracao do sensor de temperatura com resolucao de 0,1°C/LSB
    pinMode(0, INPUT);
    analogReference(INTERNAL);
    
    Serial.begin(9600); // Serial para Debug
    
    Wire.begin();       // Inicio o Wire

    /* Definicao do numero de linhas e colunas do LCD */
    lcd.begin(16,2);
    //Inicio o LCD com ele apagado
    // Obter valores inseridos na Serial e decodificá-los (caso haja algo na serial, já faz a leitura)
    fun_deco();
    lcd.clear();
    
}


void loop(){


    // Mudança do display e seu valor só ocorrerá a cada interrupção do temporizador, no caso 4 ms, isso está relacionado à variável troca
    if(troca) visor(u, d, c, m);

}

// Função que decodifica a mensagem que foi enviada ao monitor, e para o caso de setar a velocidade, retorna o valor da velocidade
long fun_deco() {
    


    // Verifica se a variavel de sinalizacao de mensagem foi setada e se passou um tempo suficiente para ele se atualizar
    if(pode_entrar) {
        
        lcd.clear();
        lcd.setCursor(0,0); // Cursor na coluna 0 e linha 0
         

        // Armazena os elementos do buffer dentro da variavel do tipo String para manipulacoes futuras
        for(int j=0; j<tam_msg;j++){
          codigo.concat(mensagem[j]);
        }


        // Verifica qual comando foi escrito no teclado, para enviar a UART sua respectiva mensagem (de erro ou nao)
        if (codigo.equals("1")) {
            lcd.print("RESET");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 1;
        }
        else if (codigo.equals("2")) {
            lcd.print("2 - STATUS");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 2;
        }
        else if (codigo.equals("3")) {
            lcd.print("3 - START");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 3;
        }
        else if (codigo.equals("4")) {
            lcd.print("4 - FIM");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 4;
        }
        
        else if (codigo.equals("5")) {
            lcd.print("5 - TRANSF. DADOS");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 5;
        }
        /*else {
            lcd.print("BEM VINDO");
            lcd.setCursor(0,1);
            lcd.print("DIGITE A FUNCAO:");
        }*/

        // Sinalizo que o último comando enviado já foi executado e está aguardando o próximo
        pode_entrar = 0;
        confirmacao();
    }
}

void confirmacao() {

    String temp;

    if (codigo == '#') {
        switch (cod_anterior)
        {
        case 1:
            /* code */
            break;
        case 2:
            lcd.print("GRAVADOS: ");
            lcd.setCursor(11,0);
            lcd.print(/*colocar a variavel de contagem da posicao da memoria*/);
            lcd.setCursor(0,1);
            lcd.print("DISPONIVEL: ");
            lcd.setCursor(13,0);
            lcd.print(/*colocar 1-(variavel de contagem da posicao da memoria)*/);
            break;
        case 3:
            /* code */
            temperatura();
            Write();
            break;
        case 4:
            /* code */
            break;
        case 5:
            lcd.print("TRANSF. DADOS");
            lcd.setCursor(0,1);
            lcd.print("QNTDE.: ");
            // Leitura do teclado aqui
            LSB = Read(2047);
            MSB = Read(2046);
            while (i<N) {
                endereco = 2*((256*MSB)+LSB);
                MSB_lido = Read(endereco-1);
                LSB_lido = Read(endereco);
                temp = (MSB_lido << 8) + LSB_lido;
                Serial.println(temp);
                i++;
            }
            Read(/*variavel da quantidade*/);
            break;
        }
    }
    else if (codigo == '*') {
        lcd.print("CANCELADO");
        lcd.setCursor(0,1);
        lcd.print("DIGITE A FUNCAO:");
    }
}



void temperatura(){

    dado = analogRead(Temp);

    Serial.println(dado*0.1, DEC);
}

void Write(byte Add, byte Data){
    if (pode_escrever) {
    vetor[0] = Add;
    vetor[1] = Data;
    Wire.beginTransmission(ADD);
    Wire.write(vetor,2);
    Wire.endTransmission();
    }

    pode_escrever = 0;
}

void Read(byte Add){
  char DATA;
  Wire.beginTransmission(ADD);
  Wire.write(Add);
  Wire.endTransmission();
  Wire.requestFrom(ADD, 1);
  //delay(1);
  if(Wire.available()){
    DATA = Wire.read(); 
    //Serial.println(DATA);
    return DATA;
  }
}


// Função que obtém valor da temperatura e apresenta nos displays utilizando a interface I2C (TWI)
void visor(int x, int y, int w, int z ){
    
    switch (contDisplay)
    {
    case 0:
        Wire.beginTransmission(32);
        Wire.write(112 + x);  // Display do decimo ligado + valor do decimo
        Wire.endTransmission();
        break;
    case 1:
        Wire.beginTransmission(32);
        Wire.write(176 + y);  // Display da unidade ligado + valor da unidade
        Wire.endTransmission(); 
        break;
    case 2:
        Wire.beginTransmission(32);
        Wire.write(208 + w);  // Display da unidade ligado + valor da unidade
        Wire.endTransmission();
        break;
    case 3:
        Wire.beginTransmission(32);
        Wire.write(224 + z);  // Display do dezena ligado + valor da dezena
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
    cont_escrita++;

    // Depois de 220 ms permitimos que tenha mudança no LCD
    if(!(cont%55)) pode_entrar = 1;

    if(!(cont_escrita%500)) {
        pode_escrever = 1;
    }

    // Indico que passou o tempo do temporizador e que podemos enviar um novo dado via I2C e trocar o display
    troca = 1;

}

/* rotina de servico de interrupcao que eh executada toda vez ha mudanca de 
estado e que mede a quantidade de vezes que essa interrupcao foi chamada */
ISR (PCINT1_vect) {
    /* cada vez que a interrupcao eh chamada, o valor de n eh incrementado
       mas levando em conta que o PIN CHANGE eh executado quando ha variacao de 
       nivel logico, isto eh, quando for calculado a frequencia estimada, 
       o valor de n deve ser dividido por 2 */
    n++;
}
