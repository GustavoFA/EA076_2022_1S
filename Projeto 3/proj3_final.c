// Projeto 3 - GRAVADOR DE DADOS AMBIENTAIS
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

// Usamos Timer 0 -> 4 ms


// Bibliotecas 
#include <Wire.h>
#include <LiquidCrystal.h>
#define L1 13
#define L2 10
#define L3 2
#define L4 9
#define C1 4
#define C2 3
#define C3 A5

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

// --------------------
// Teclado Matricial

// Status para definir qual linha estará acionada
unsigned int status_l = 0;  

// Status para definir qual coluna foi acionada
unsigned int status_c = 0; // 0 indica que não foi acionada nenhuma coluna

// Variável para habilitar a checagem dos botões
bool deb = 0;

// Contador para os botões (usados para o debounce)
byte cont_c1 = 0;
byte cont_c2 = 0;
byte cont_c3 = 0;

// Variáveis para armazenamento do estado atual das colunas
bool d_C1 = 0;
bool d_C2 = 0;
bool d_C3 = 0;

// Armazenar ultimo caracter apresentado no LCD
char ult_char = NULL;

// Armazerna o caracter do respectivo botão pressionado
char Car = NULL;

char cont_pres = 0;

char pos_lcd = 0;

// ---------------------------

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

    // Configuracao do teclado matricial
    // Linhas serão acionadas e colunas serão lidas
    pinMode(L1, OUTPUT);
    pinMode(L2, OUTPUT);
    pinMode(L3, OUTPUT);
    pinMode(L4, OUTPUT);

    // Deixo as linhas desativadas na inicialização
    digitalWrite(L1, 0);
    digitalWrite(L2, 0);
    digitalWrite(L3, 0);
    digitalWrite(L4, 0);

    // Colunas serão habilitadas como entradas 
    pinMode(C1, INPUT);
    pinMode(C2, INPUT);
    pinMode(C3, INPUT);

    // Com isso, teremos a estratégia de ficar alternando entre as linhas e varrer as colunas verificando qual botão foi pressionado.


    /* Definicao do numero de linhas e colunas do LCD */
    lcd.begin(16,2);
    //Inicio o LCD com ele apagado
    // Obter valores inseridos na Serial e decodificá-los (caso haja algo na serial, já faz a leitura)
    fun_deco();
    lcd.clear();
    
}


void loop(){

    // Verifico o teclado
    teclado();

    // Caso tenha algum caracter pressionado diferente do ultimo visualizado
    if(Car != ult_char){
    // Implementar função do LCD aqui (no lugar do print)
        if(Car > 0){

            printlcd(Car);
        }
        
        // Salvo o último valor printado
        ult_char = Car;
    }

    // Mudança do display e seu valor só ocorrerá a cada interrupção do temporizador, no caso 4 ms, isso está relacionado à variável troca
    //if(troca) visor(u, d, c, m);

}

// Função que decodifica a mensagem que foi enviada ao monitor, e para o caso de setar a velocidade, retorna o valor da velocidade
long fun_deco() {
    


    // Verifica se a variavel de sinalizacao de mensagem foi setada e se passou um tempo suficiente para ele se atualizar
    if(pode_entrar) {
        
        lcd.clear();
        lcd.setCursor(0,0); // Cursor na coluna 0 e linha 0


        // Verifica qual comando foi escrito no teclado, para enviar a UART sua respectiva mensagem (de erro ou nao)
        if (Car == "1")) {
            lcd.print("RESET");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 1;
        }
        else if (Car == "2") {
            lcd.print("2 - STATUS");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 2;
        }
        else if (Car == "3") {
            lcd.print("3 - START");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 3;
        }
        else if (Car == "4") {
            lcd.print("4 - FIM");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 4;
        }
        
        else if (Car == "5") {
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

    if (Car == '#') {
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

// Função para o teclado matricial
void teclado(){

    // Alternância entre a linha acionada ocorre no tempo do laço 
    switch (status_l){

        case 0:
            digitalWrite(L1, 1);
            digitalWrite(L4, 0);
            break;
        case 1:
            digitalWrite(L2, 1);
            digitalWrite(L1, 0);
            break;
        case 2:
            digitalWrite(L3, 1);
            digitalWrite(L2, 0);
            break;
        case 3:
            digitalWrite(L4, 1);
            digitalWrite(L3, 0);
            break;

    }

    // Armazenamento do estado atual de cada coluna da respectiva linha
    d_C1 = digitalRead(C1);
    d_C2 = digitalRead(C2);
    d_C3 = digitalRead(C3);
    
    // A cada 4 ms checo se alguma coluna foi acionada
    if(deb){

        // Contador para estado sem botão pressionado 
        cont_pres++;

        // Zero a variável deb para ficar alternando e fazer a condição ser válida apenas a cada 4 ms
        deb = 0;

        // Faço contagem de vezes que tal coluna esteja em estado alto para usar como estratégica para o debounce
        if(d_C1){
            cont_c1++;
        }else if(d_C2){
            cont_c2++;
        }else if(d_C3){
            cont_c3++;
        }

        // Verifico se alguma das colunas ficou mais de 52 ms pressionada e converto o botão pressionado em seu caracter
        // Zero o contador cont_pres, pois verificamos que algum botão foi pressionado
        // Indico qual coluna foi verificada como em alto
        // Zero as contagens de vezes que as colunas foram lidas como alta
        // Faço a "tradução" da linha e coluna em um caracter

        if(cont_c1 > 6){
            cont_pres = 0;
            status_c = 1;
            cont_c1 = cont_c2 = cont_c3 = 0;
            tradutor(status_l, status_c);
        }else if(cont_c2 > 6){
            cont_pres = 0;
            status_c = 2;
            cont_c1 = cont_c2 = cont_c3 = 0;
            tradutor(status_l, status_c);
        }else if(cont_c3 > 6){
            cont_pres = 0;
            status_c = 3;
            cont_c1 = cont_c2 = cont_c3 = 0;
            tradutor(status_l, status_c);
        }
        
        // Esse último estado serve para após um tempo que ficar sem nenhum botão ser pressionado o microcontrolador identificar tal estado
        // Esse estado serve para solucionar o problema de um botão ficar pressionado e printando no LCD
        else if(cont_pres > 40){
            cont_pres = 0;
            status_c = 0;
            tradutor(status_l, status_c);
        }
        
    }
    
    // Mudo a posição da linha
    status_l++;
    if(status_l > 3) status_l = 0;

}

// Função para traduzir a leitura do botão para o caracter
void tradutor(unsigned int lin, unsigned int col){

    // Faço uma equação para gerar valores decimais unicos para cada digito do teclado
    unsigned int fator = 10*lin + col;

    switch (fator)
    {
    case 1:
        Car = '1';
        break;
    case 2:
        Car = '2';
        break;
    case 3:
        Car = '3';
        break;
    case 11:
        Car = '4';
        break;
    case 12:
        Car = '5';
        break;
    case 13:
        Car = '6';
        break;
    case 21:
        Car = '7';
        break;
    case 22:
        Car = '8';
        break;
    case 23:
        Car = '9';
        break;
    case 31:
        Car = '*';
        break;
    case 32:
        Car = '0';
        break;
    case 33:
        Car = '#';
        break;
    default:
        Car = NULL;
        break;
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

    // Habilita checagem da coluna
    deb = 1;

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
