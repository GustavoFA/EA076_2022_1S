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

/*

    Debounce de 50 ms
    Mudança de linha a cada 200 ms
    Varredura nas colunas

*/

// Variável para contagem de tempo
byte cont = 0;

// Status para definir qual linha estará acionada
byte status_l = 0;

// Status para definir qual coluna foi acionada
byte status_c = 0; // 0 indica que não foi acionada nenhuma coluna

// Detectou o acionadmento de algum botão
bool detect = 0;

// Status da coluna para confirmar depois do debounce
byte status_c_deb = 0;

// Contador de tempo para debounce
byte cont_deb = 0;


void setup(){

  Serial.begin(9600);
  
  // Linhas serão acionadas e colunas serão lidas
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);

  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);


  configuracao_Timer0();

}

void loop(){
  
  teclado();

}

void teclado(){

    if(!detect){
        // Definir a linha acionada a cada 200 ms
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
    
        // Leitura das colunas
        bool d_C1 = digitalRead(C1);
        bool d_C2 = digitalRead(C2);
        bool d_C3 = digitalRead(C3);


        if(d_C1){
            status_c = 1;
            detect = 1;
        }else if(d_C2){
            status_c = 2; 
            detect = 1;
        }else if(d_C3){
            status_c = 3;
            detect = 1;
        }else{
            status_c = 0;
        }

    }else{
        // depois de 20 ms checo novamente se o botão ainda está pressionado
        if(cont_deb > 12){

            // Leitura das colunas
            bool d_C1 = digitalRead(C1);
            bool d_C2 = digitalRead(C2);
            bool d_C3 = digitalRead(C3);

            if(d_C1){
                status_c_deb = 1;
            }else if(d_C2){
                status_c_deb = 2; 
            }else if(d_C3){
                status_c_deb = 3;
            }else{
                status_c_deb = 0;
            }

            if(status_c == status_c_deb){
                Serial.print("Linha: ");
                Serial.println(status_l);
                Serial.print("Coluna: ");
                Serial.println(status_c);
            }

            cont_deb = 0;
            detect = 0;

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

    if(cont >= 50){
        status_l++;
        if(status_l > 3) status_l = 0;
        cont = 0;
    }

    if(detect){
        cont_deb++;
    }
}
