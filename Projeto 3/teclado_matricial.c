// Atividade Parcial 2 - Teclado Matricial
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

// Link Tinkercad: https://www.tinkercad.com/things/cTmmGuVzZMp-terrific-trug/editel?sharecode=gE3_BYuuf9oVNRcQMq7H1X0me_c7ptIK3DblF9cNT2c

#define L1 9
#define L2 8
#define L3 7
#define L4 6
#define C1 5
#define C2 4
#define C3 3


// Variável para contagem de tempo
byte cont = 0;

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

  // Configuração do temporizador de 4 ms
  configuracao_Timer0();

}

void loop(){
  
  teclado();

}

// Função para o teclado matricial
void teclado(){

    // Alternância entre a linha acionada ocorre a cada 160 ms
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

        deb = 0;
        

        // Faço contagem de vezes que tal coluna esteve em estado alto para usar como estratégica para o debounce
        if(d_C1){
            cont_c1++;
        }else if(d_C2){
            cont_c2++;
        }else if(d_C3){
            cont_c3++;
        }

        // Após um tempo maior que 48 ms confirmo se alguma das colunas foi acionada
        if(cont_c1 > 12){
            status_c = 1;
            cont_c1 = 0;
            print();
        }else if(cont_c2 > 12){
            status_c = 2;
            cont_c2 = 0;
            print();
        }else if(cont_c3 > 12){
            status_c = 3;
            cont_c3 = 0;
            print();
        }else{
            status_c = 0;
        }

    }
    

}

// Função de print rápido para teste
void print(){

    Serial.print("L: ");
    Serial.println(status_l);
    Serial.print("C: ");
    Serial.println(status_c);

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

    // Contador para as linhas
    cont++;

    // Após 160 ms muda a linha e reseta
    if(cont >= 40){
        status_l++;
        if(status_l > 3) status_l = 0;
        cont = 0;
    }

    // Habilita checagem da coluna
    deb = 1;

}
