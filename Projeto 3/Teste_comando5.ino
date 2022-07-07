/*
Programa para testar o comando 5 - consiste em, eu salvo caracteres da tabela ASCII nas posicoes de 0 a 255 na memoria, considerando apenas 1 pagina (0x50 = 80)
e atingido o limite maximo, eu uso o comando 5 para mostrar uma certa quantidade dos caracteres salvos. Toda vez que for carregar o programa, tem q zerar a memoria
*/

// Bibliotecas 
#include <Wire.h>
#include <LiquidCrystal.h>

// ------ Defines -------------

#define Temp A0
#define L1 13
#define L2 10
#define L3 2
#define L4 9
#define C1 4
#define C2 3
#define C3 A1 

// variável auxiliar para comunição I2C ocorrer a cada interrupção do temporizador
bool troca = 1; 

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

// Contador de dados gravados (conta cada temperatura foi gravado, ou seja, a cada 2 endereços alocados)
unsigned int dado_grav = 0;

// Permite escrita na memória e auxilia na temporização
bool escrita = 1;

// Variável para selecionar estado da Maq. de estados finitos
byte estado = 1;

// ---------------------------

// Variavel da temperatura (16 bits)
unsigned int dado;
byte endereco;
byte pagina;
char num = 33;

// Variável para auxiliar na seleção do comando
int cod_anterior = 0;

// 
bool leit_quant = 0;

// Variável para determinar a quantidade de posições que serão lidas da memória
char quanti[4];

// Identifica quantos algarismos tem o valor inserido de quantidade
int tam_quanti = 0;

// Contador para selecionar a posição da página que será lido e inserido na Serial
byte pos_print = 0;

// Contador para selecionar a pág que estará sendo lida para ser inserida na Serial
byte pag_print = 80;

// Habilita as impressões na Serial
bool serial_pode = 0;
bool sair = 0;

// 
unsigned int valor_final_alg;

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
    Serial.println(ponteiro[0]);
    Serial.println(ponteiro[1]);

    // Divido por 2 pois a cada 2 posições na memória correspondem a 1 dado gravado
    // Já atualizo o valor de dados gravados
    dado_grav = (256*ponteiro[0] + ponteiro[1])/2;
}

void loop(){
  // Leitura do teclado (debounce incluso)
  teclado();

  // Caso esteja habilitado para imprimir na Serial aqui será feito
  if(serial_pode) print_serial();

  // Caso tenha algum caracter pressionado diferente do ultimo visualizado
  if(Car != ult_char){
    // Implementar função do LCD aqui (no lugar do print)
    if(Car > 0){
        // Caso o comando 5 não esteja selecionado
        if(!leit_quant){
          primeira_leitura();
        }
        // Caso o comando 5 seja selecionado
        else{
        segunda_leitura();
        }
    }
    // Salvo o último valor printado
    ult_char = Car;
  }

    endereco = 0;
    pagina = 80;
    if(endereco<0xFF && sair == 0 && ler_temp == 1) {
        Write(endereco, pagina, num);
        //Read(t);
        endereco++;
        num++;
    }
    else {
        escrita = 0;
        endereco = 0;
        sair = 1;
    }
}

// Tempo de ciclo de escrita é de 5ms, mas estamos utilizando um tempo maior de 16 ms
// Parâmetros são respectivamente: Endereço (0 - 255), página (0x50 - 0x57 = 80 - 87) e Data = dado temp.
void Write(byte Add, byte pag, byte Data){

    // A memória não reseta sem o usuário permitir. Se atingir limite ela fica parada
    if(dado_grav < 1022){
        vetor[0] = Add;   // Byte de endereço
        vetor[1] = Data;  // Byte de dado
        Wire.beginTransmission(pag);
        Wire.write(vetor,2);
        Wire.endTransmission();
    }

}

// Parâmetros semelhantes a função de escrita
char Read(byte Add, byte pag){

  // Variável local para armazenar valor lido
  char DATA;

  Wire.beginTransmission(pag); 
  Wire.write(Add);  
  Wire.endTransmission();
  Wire.requestFrom(pag, 1);
  if(Wire.available()){
    DATA = Wire.read(); 
    //Serial.println(DATA);
    return DATA;
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

        // Condições seguintes identificam cada comando

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
            // Caso o valor inserido não represente nenhum comando será habilitado que um próximo valor seja lido 
            pode_entrar = 1;
            
        }
    }else{
        // Confirmações do comando 
        confirmacao();
    }
}

// Função para checar a confirmação/cancelamento do comando inserido
// Responsável também pela execução de algumas ações ou de dar a partida para a resolução de alguns comandos
void confirmacao() {

    // Confirmação do comando
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

            // Reseto variáveis de controle
            pode_entrar = 1;
            cod_anterior = 0;
            break;
        case 3:

            // Habilito para que a cada leitura da temperatura seja armazenado o valor na memória
            escrita = 1;

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("INICIO DA");
            lcd.setCursor(0,1);
            lcd.print("COLETA DE TEMP.");

            // Reseto variáveis de controle
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
  
            // Reseto variáveis de controle
            pode_entrar = 1;
            cod_anterior = 0;
            break;
        case 5:

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("TRANSF. DADOS");
            lcd.setCursor(0,1);
            lcd.print("QNTDE.: ");

            // Habilito a leitura da quantidade de dados que serão transf.
            leit_quant = 1;
            
            break;
        }
    }
    // Cancelamento
    else if (Car == '*') {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("CANCELADO");
        lcd.setCursor(0,1);
        lcd.print("DIGITE A FUNCAO:");
        
        // Reseto variáveis de controle
        pode_entrar = 1;
        cod_anterior = 0;
    }
}

// 2 controles: Zerar e Incrementar
void cont_mem(char par){

    // Caso em que devemos zerar a memória
    if(par == 'z'){
        // Zera ponteiros da memoria
        ponteiro[0] = ponteiro[1] = 0;
        // Zera variável de dados gravados
        dado_grav = 0;

        // Estado 5 da Máq. de Estados corresponde à ação de guardar o ponteiro
        estado = 5;

    }
    // Caso em que devemos incrementar a memória (salvando algo nela)
    else if(par == 'i'){

        // Para evitar a escrita em locais não indicados ou o reset da memória sem autorização do usuário
        if(dado_grav < 1022) {

            // Verifica se já atingiu limiar da página
            if(ponteiro[1] >= 255){
                ponteiro[0]++;
                ponteiro[1] = 0;
            }else{
                
                ponteiro[1]++;
            }

        }

    }
}

// Função para impressão na Serial
void print_serial(){

  // Condição para quantidade desejada
  if(pos_print < valor_final_alg){

    // Armazeno o MSB do dado
    byte MSB_serial;
    MSB_serial = Read(pos_print, pag_print);
    Serial.println(MSB_serial);
    // Passo para a próxima posição tomando cuidado com o limite da página
    pos_print++;
    if(pos_print > 255){
      pos_print = 0;
      pag_print++;
    }
    
    // Armazeno o LSB do dado
    byte LSB_serial;
    LSB_serial = Read(pos_print, pag_print);
    Serial.println(LSB_serial);
    pos_print++;
    if(pos_print > 255){
      pos_print = 0;
      pag_print++;
    }
    // Concateno os valores armazenados de LSB e MSB 
    unsigned int local;
    local = (unsigned int) ((MSB_serial << 8)|(LSB_serial));
   
    // Imprimo na Serial com a quebra de linha
    Serial.println(local*0.1  , DEC);
    //MSB_serial = 0;
    //LSB_serial = 0;
    local = 0;

    
  }else{

    // Caso tenha impresso os dados das posições desejadas reseto as variáveis
    serial_pode = 0;
    pag_print = 80;
    pos_print = 0;
    leit_quant = 0;
    pode_entrar = 1;
    cod_anterior = 0;
    tam_quanti = 0;
    
  }
}

// Função para checagem do comando 5
void segunda_leitura(){

  // Verifico a confirmação da quantidade
  if(Car == '#'){

    // Executo o comando com a quantidade já armazenada
    

    unsigned int alg_aux[4];

    // Conversão dos caracteres em um valor inteiro
    for(int i = 0; i < tam_quanti; i++){
      alg_aux[i] = quanti[i] - 48;
    }

    if(tam_quanti == 1){
      valor_final_alg = alg_aux[0];
      serial_pode = 1;
    }else if(tam_quanti == 2){
      valor_final_alg = alg_aux[0]*10 + alg_aux[1];
      serial_pode = 1;
    }else if(tam_quanti == 3){
      valor_final_alg = alg_aux[0]*100 + alg_aux[1]*10 + alg_aux[2];
      serial_pode = 1;
    }else if(tam_quanti == 4){
      valor_final_alg = alg_aux[0]*1000 + alg_aux[1]*100 + alg_aux[2]*10 + alg_aux[3];
      serial_pode = 1;
    }else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("VALOR INVALIDO");
      lcd.setCursor(0,1);
      lcd.print("DIGITE A FUNCAO:");
  
  
      leit_quant = 0;
      pode_entrar = 1;
      cod_anterior = 0;
  
      tam_quanti = 0;
    }
    
  }
  // Cancelo o comando resetando as variáveis
  else if(Car == '*'){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("CANCELADO");
    lcd.setCursor(0,1);
    lcd.print("DIGITE A FUNCAO:");


    leit_quant = 0;
    pode_entrar = 1;
    cod_anterior = 0;

    tam_quanti = 0;
    
  }
  // Enquanto não houver confirmação ou cancelamento verifico a entrada de algum valor para representar a quantidade
  else{

     // Armazeno no vetor de caracteres
     quanti[tam_quanti] = Car;

     // Guardo a quantidade de caracteres
     tam_quanti++;

     // Imprimo no LCD para facilitar a visualização para o usuário
     lcd.setCursor(7 + tam_quanti, 1);
     lcd.print(Car);
     
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

    // Tempo para checagem das colunas
    deb = 1;

    // Variável para habilitar a troca de display
    troca = 1;
    cont_2s++;
    if(cont_2s >= 2){
        ler_temp = 1;
        cont_2s = 0;
    }
}

