// Atividade Parcial 2 - Teclado Matricial
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

// Link Tinkercad: https://www.tinkercad.com/things/cTmmGuVzZMp-terrific-trug/editel?sharecode=gE3_BYuuf9oVNRcQMq7H1X0me_c7ptIK3DblF9cNT2c

#include <LiquidCrystal.h>

#define L1 13
#define L2 10
#define L3 2
#define L4 9
#define C1 4
#define C2 3
#define C3 A5

LiquidCrystal lcd(12, 11, 5, 7, 6, 8); // Definicao dos pinos que serao utilizados para a ligacao do display

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


void setup(){

  Serial.begin(9600);
  
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
  lcd.clear();  

  // Configuração do temporizador de 4 ms
  configuracao_Timer0();

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
}

// Função para printar no LCD
void printlcd(char g){

    lcd.setCursor(pos_lcd,1);
    lcd.print(g);
    pos_lcd++;
    if(pos_lcd > 15) pos_lcd = 0;

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

    // Habilita checagem da coluna
    deb = 1;

}
