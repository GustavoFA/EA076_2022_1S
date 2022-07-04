// Projeto 3 - GRAVADOR DE DADOS AMBIENTAIS
// Gustavo Freitas Alves          236249
// Jitesh Ashok Manilal Vassaram  175867

/*
    * Sensor de temperatura a cada 2s  (OK)
    * Displays de 7 segmentos          (OK)
    * LCD                              (OK)
    * Teclado                          (OK)
    * Memória
    * Comandos

*/

// Usamos Timer 0 -> 4 ms


// Bibliotecas 
#include <Wire.h>
#include <LiquidCrystal.h>

#define ADD 0x50    // 0101 0000 - ENDERECO DO 24c16
#define Temp A0

#define L1 13
#define L2 10
#define L3 2
#define L4 9
#define C1 4
#define C2 3
#define C3 A1 

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

// Habilita a leitura do sensor
bool ler_temp = 0;

// Contador de 2 segundos utilizado pelo sensor de temperatura
int cont_2s = 0;

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

// Contador para tempo em que nenhum botão foi pressionado
char cont_pres = 0;

// Contador para posicionar os digitos na coluna do LCD
char pos_lcd = 0;

// Variável que irá bloquear o LCD de ficar frequentemente atualizando-se 
bool pode_entrar = 1;

//  -------- memória ---------

/* Memoria
 *  2048 endereços de 8 bits
 *  3 paginas: 0x50 a 0x57 (80 a 87) + 8 bits(0 a 255)
 */

// Endereço do ponteiro: 0x57FE(87 e 254) e 0x57FF (87 e 255)

// Vetor para cada posição do ponteiro da memória
byte ponteiro[2]; // 0 = MSB e 1 = LSB

// Vetor para endereço + dado
byte vetor[2];

// Variável para possibilitar que a escrita ocorra a cada 5 ms
bool pode_escrever = 0;

// Contador para tempo de espera de 8 ms para cada escrita
byte cont_escrita = 0;

// Contador de dados gravados
unsigned int dado_grav = 0;

// Permite escrita na memória
bool escrita = 0;

byte estado = 0;

// ---------------------------

// Variavel da temperatura
unsigned int dado;

int cod_anterior = 0;


// ------------------------------------------

void setup(){

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

    configuracao_Timer0(); // Configura o temporizador Timer 0 -> 4 ms

    // Configuracao do sensor de temperatura com resolucao de 0,1°C/LSB
    pinMode(Temp, INPUT);

    // Tensão interna de referência de 1.1V
    analogReference(INTERNAL);
    
    Serial.begin(9600); // Serial para Debug
    
    Wire.begin();       // Inicio o Wire

    /* Definicao do numero de linhas e colunas do LCD */
    lcd.begin(16,2);
    lcd.clear();
    lcd.print("BEM VINDO");
    lcd.setCursor(0,1);
    lcd.print("DIGITE A FUNCAO:");

    // Verifico a posição da última memória disponível
    ponteiro[0] = Read(254, 87);
    ponteiro[1] = Read(255, 87);

    dado_grav = 256*ponteiro[0] + ponteiro[1];

    Serial.println(ponteiro[0]);
    Serial.println(ponteiro[1]);
    
}


void loop(){

  teclado();

  // Caso tenha algum caracter pressionado diferente do ultimo visualizado
  if(Car != ult_char){
    // Implementar função do LCD aqui (no lugar do print)
    if(Car > 0){
        primeira_leitura();
    }
    // Salvo o último valor printado
    ult_char = Car;
  }

  if(ler_temp) temperatura();

  salvar_dado();

  if(troca) visor(u, d, c, m);


}

// Função para medição da temperatura
void temperatura(){

    ler_temp = 0;

    // Obtenho o valor digital do sensor
    dado = analogRead(Temp);

    // Obtenho cada digito sendo m dezena, c unidade, d decimo e u centesimo
    m = (int) (dado / 100);
    c = (int) (dado - 100*m)/10;
    d = (int) (dado - 100*m - 10*c);

    if(escrita) estado = 1;

}

/* Memoria
 *  2048 endereços de 8 bits
 *  3 paginas: 0x50 a 0x57 (80 a 87) + 8 bits(0 a 255)
 */

// Função de escrita
// Tempo de ciclo de escrita é de 5ms, mas estamos utilizando um tempo maior de 16 ms
void Write(byte Add, byte pag, byte Data){

    if(dado_grav < 1022){
        vetor[0] = Add;   // Byte de endereço
        vetor[1] = Data;  // Byte de dado
        Wire.beginTransmission(pag);
        Wire.write(vetor,2);
        Wire.endTransmission();
    }

}

// Função para contabilizar e gerenciar as variáveis de controle da memória
// Ela funciona
void cont_mem(char par){

    // Caso em que devemos zerar a memória
    if(par == 'z'){
        // Zera ponteiros da memoria
        ponteiro[0] = ponteiro[1] = 0;
        dado_grav = 0;
        //estado = 3;

    }
    // Caso em que devemos incrementar a memória (salvando algo nela)
    else if(par == 'i'){

        // Caso continue salvando além do espaço disponível (começa a sobre escrever)
        if(dado_grav < 1022) {

            // Verifica se já atingiu limiar da pag
            if(ponteiro[1] >= 255){
                ponteiro[0]++;
                ponteiro[1] = 0;
            }else{
                // Verifica se já atingiu limiar da memoria
                ponteiro[1]++;
            }

        }

    }
}

// Função de leitura
// pag: 80 a 87
// Add: 0 a 255
char Read(byte Add, byte pag){

  char DATA;

  Wire.beginTransmission(pag); 
  Wire.write(Add);  
  Wire.endTransmission();
  Wire.requestFrom(pag, 1);
  if(Wire.available()){
    DATA = Wire.read(); 
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

        // Verifico se alguma das colunas ficou mais de 24 ms pressionada e converto o botão pressionado em seu caracter
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
        }
        
        else if(cont_c3 > 6){
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

// Função que decodifica a mensagem que foi enviada ao monitor, e para o caso de setar a velocidade, retorna o valor da velocidade
void primeira_leitura() {
    
    // Verifica se a variavel de sinalizacao de mensagem foi setada e se passou um tempo suficiente para ele se atualizar
    if(pode_entrar) {

        // Sinalizo que o último comando enviado já foi executado e está aguardando o próximo
        pode_entrar = 0;

        // Verifica qual comando foi escrito no teclado, para enviar a UART sua respectiva mensagem (de erro ou nao)
        if (Car == '1') {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("1 - RESET");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 1;
        }
        else if (Car == '2') {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("2 - STATUS");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 2;
        }
        else if (Car == '3') {
            lcd.clear();
            lcd.setCursor(0,0); 
            lcd.print("3 - START COLETA");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 3;
        }
        else if (Car == '4') {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("4 - FIM COLETA");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 4;
        }
        
        else if (Car == '5') {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("5 - TRANSF.DADOS");
            lcd.setCursor(0,1);
            lcd.print("CONFIRMAR?");
            cod_anterior = 5;
        }else {
            // Como o comando mandado não tem ele pode entrar novamente
            pode_entrar = 1;
            
        }
    }else{
        confirmacao();
    }
}

void confirmacao() {

    if (Car == '#') {
        switch (cod_anterior)
        {
        case 1:
            /* apagar toda a memória */

            // Zero os ponteiros
            cont_mem('z');

            // Apresento no LCD
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("MEMORIA RESETADA");

            // Reseto variáveis de controle
            pode_entrar = 1;
            cod_anterior = 0;
            
            break;
        case 2:

            // Apresento número de medidas salvas
            lcd.clear();
            lcd.print("GRAVADOS: ");
            lcd.setCursor(12,0);
            lcd.print(dado_grav);

            // Apresento número de medidas que podem ser guardadas
            lcd.setCursor(0,1);
            lcd.print("DISPONIVEL: ");
            lcd.setCursor(12,1);
            lcd.print((unsigned int) (1022 - dado_grav));

            pode_entrar = 1;
            cod_anterior = 0;
            break;
        case 3:

            // Habilito para que a cada leitura seja armazenado o valor na memória
            escrita = 1;

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("INICIO DA");
            lcd.setCursor(0,1);
            lcd.print("COLETA DE TEMP.");

            pode_entrar = 1;
            cod_anterior = 0;
            break;
        case 4:

            // Desabilito que ocorra a escrita na memória
            escrita = 0;

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("COLETA DE TEMP.");
            lcd.setCursor(0,1);
            lcd.print("FINALIZADA");
  
            pode_entrar = 1;
            cod_anterior = 0;
            break;
        case 5:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("TRANSF. DADOS");
            lcd.setCursor(0,1);
            lcd.print("QNTDE.: ");
            // Faz transferencia de dados
            // Só volta a poder entrar quando acabar a ação do comando
            pode_entrar = 1;
            cod_anterior = 0;
            break;
        }
    }
    else if (Car == '*') {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("CANCELADO");
        lcd.setCursor(0,1);
        lcd.print("DIGITE A FUNCAO:");
        

        pode_entrar = 1;
        cod_anterior = 0;
    }
}


// Rotina de servico de interrupcao do temporizador de 4 ms
ISR(TIMER0_COMPA_vect){

    // Tempo para checagem das colunas
    deb = 1;

    // contador para tempo de 2s para leitura da temperatura
    cont_2s++;
    if(cont_2s >= 500){
        ler_temp = 1;
        cont_2s = 0;
    }
     
    // Tempo para escrita do LSB de 8 ms
    // Essa condição ele entra
    //if((estado == 2) || (estado == 3) || (estado == 4)){
      if(estado > 1){
        cont_escrita++;
        if(cont_escrita > 1){
            pode_escrever = 1;
            cont_escrita = 0;
        }
    }

    // Variável para habilitar a troca de display
    troca = 1;

}

// Máquina de estados finitos para armazenamento da memória 
/*
  Salva na sequência: MSB, LSB, posição da memória
 */

 // Testei diversas vezes e o que ocorre é: ele entra na máquina, mas só faz o estado 1. A variável de estado muda para 2, mas continua só ocorrendo o caso 1.
void salvar_dado(){

    switch (estado)
    {
    case 0:
        // Não faz nada
        break;
    
    case 1:
        // salvo o MSB
        byte MSB = (byte) (dado >> 8);
        Write(ponteiro[1], ponteiro[0], MSB);
        cont_mem('i');
        estado = 2;
        pode_escrever = 0;
        
        break;

    case 2:

        if(pode_escrever){
            // salvo o LSB
            byte LSB = (byte) (0x00FF & dado);
            Write(ponteiro[1], ponteiro[0], LSB);
            cont_mem('i');
            dado_grav++;
            estado = 3;
            pode_escrever = 0;
            
        }
        break;

    case 3:
        if(pode_escrever){
            Write(254, 87, ponteiro[0]);
            estado = 4;
            pode_escrever = 0;
            
        }
        break;

    case 4:
        if(pode_escrever){
            Write(255, 87, ponteiro[1]);
            estado = 0;
            pode_escrever = 0;
            
        }
        break;
        
    }

}
