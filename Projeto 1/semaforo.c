// Projeto 1 - Semáforo para pedestres
// Gustavo Freitas Alves    236249

      /*  Introdução

          Nesse projeto estaremos desenvolvendo um semáforo composto por iluminações para veículos e para pedestres e um botão. Ele terá um
        funcionamento específico para noite, em que ficará piscando os leds vermelho para pedestre e amarelo para carros, e para dia, em que
        ficará normalmente com o verde ligado para os carros e vermelho para os pedestres, até algum pedestre pressionar o botão e uma máquina
        de estados finitos iniciar. Após a finalização da máquina de estados ele retorna ao funcionamento especifico para cada faixa do dia. 
        Essa faixa é identificada por um sensor LDR. O intuito do projeto é a construção de um semáforo inteligente, capaz de otimizar seu 
        funcionamento para cada momento do dia baseado no fluxo de carros e pessoas.

      */

// Pinagem para LDR
#define LDR A0

// ------- Variáveis

// variável para não permitir que as ações quando apertem o botão não parem quando identificar que ficou de noite
bool exec_da_parada = 0;

// contador geral do temporizador
int cont_geral = 0;

// contador de estados para a máquina de estados
byte cont_maq = 0;

// variável para permitir a troca de estado
bool prox_est = 0;

// Status do botão do semáforo
bool status_bot = 0;

// variável para identificar quando está começando as ações para os pedestres passarem
bool inicio_do_ped = 1;

// variável para conta um tempo em que o led amarelo irá ficar aceso
bool timer_amarelo = 0;

// contador para tempo de 960 ms da luz amarela (que será usado no começo das ações de travessia dos pedestres)
byte cont_960 = 0;

// vetor para salvar 10 valores de leitura do ADC
int vet_ADC[10];     

// Média dos valores registrados pelo ADC/LDR
unsigned int val_ADC;       

// variável que permite a função LDR registrar um novo valor no vetor de medidas de intensidade solar
bool timer_LDR = 0;

// variável para armazenar se está dia (= 1) ou noite (= 0)
bool esta_dia = 0;  

// variável para guardar o valor que deve ser inserido no display dos veículos
byte reg_disp_v;

// variável para guardar o valor que deve ser inserido no display dos pedestres
byte reg_disp_p;

// variável para selecionar qual display estará ativo (= 0 -> pedestres, = 1 -> veículos)
bool sel_disp = 0;

// variável para permitir alternância de qual estará ligado entre os displays
bool troca_disp = 0;

// variável para auxiliar na contagem de tempo para mudar o estado da máquina de estados
unsigned int cont_disp = 0;

// variável para identificar e auxiliar na contagem dos elementos do vetor de registros do LDR
byte aux_ADC = 0;

// contador para as luzes do semáforo piscarem
int cont_pisca = 0;

// variável que permite as Luzes semáforo piscarem
bool pisca = 0;

// variável para identificar se as luzes devem ficar acesas (=1) ou apagadas (=0) na piscagem
bool estado_pis = 1;

//variável que identifica que acabou a máquina de estados
bool fim_maq = 0;

// -------------------------------


// RSI - Interrupção botão
ISR(PCINT2_vect){ 

  // Utilizo o registrador PIND para verificar o nível da I/O PD6
  // Uso uma variável auxiliar para salvar o atual valor da posição do PD6 do registrador PIND
  byte aux_PIND = PIND & 0x40;
  // Uso o bitshift right para deslocar os bits e assim o valor da variável ser o bit menos significativo
  aux_PIND = aux_PIND >> 6;
  // Checo se o valor do bit é alto (caso esteja apertado) e se já está dia (variável atualizada pela função do LDR)
  if(aux_PIND && esta_dia){
    status_bot = 1; // caso as condições anteriores sejam satisfeitas, registro na variável que o botão foi pressionado
  }

}


// RSI - Temporizador - 8ms
ISR(TIMER0_COMPA_vect){
  
  cont_geral++;

  // verifico pela variável troca_disp se já ocorreu as ações iniciais quando apertam o botão -> Isso gera um período de 8ms para cada display
  if(troca_disp) sel_disp ^= 1; 
  // inicio a contagem apenas quando dar início nas ações geradas por apertarem o botão
  if(timer_amarelo){
    cont_960++;         // contador de 960 ms
  }

  // verifico se está noite (=0) para poder piscar os leds noturnos a cada 500 ms
  if(!esta_dia){
    cont_pisca++;
    if(!(cont_pisca % 62)) pisca = 1;      // contador de 500 ms
  }

  // Contagem para tempo de cada medição do LDR
  if(!(cont_geral % 14)) timer_LDR = 1;  // contador de 112 ms


  // contador para a mudança de estado para a contagem no display e para os leds piscarem
  // somente inicia a contagem quando acabar as ações iniciais 
  if(troca_disp){
    cont_disp++;
    if(!(cont_disp % 63)) prox_est = 1; // contador de 504 ms ~ 0.5 s
  }
  
  // Após 8 segundos reseto o valor da variável de contagem
  if(cont_geral > 1000) cont_geral = 0; 


}





void setup() {

  // Inicializo a serial (para debug)
  Serial.begin(9600);
  
  // Configuração da entrada analógica do LDR
  pinMode(LDR, INPUT);  // bloco do fluxograma [1]

  // Configuração dos LEDs dos semáforos
  // bloco do fluxograma [2]
  DDRC |= 0x3E;   // DDRC - - 1 1 1 1 1 0
  // Inicio com o semáforo vermelho para os pedestres e verde para os carros
  PORTC |= 0x18;  // PORTC - - 0 1 1 0 0 -
  PORTC &= 0x18;
  // -------------------------------------


  //  ---- Configurando as saídas do Display
  // bloco do fluxograma [3]
  DDRB |= 0x0F;   // DDRB - - - - 1 1 1 1
  DDRD &= 0xBF;
  DDRD |= 0x30;   // DDRD - 0 1 1 - - - -
  PORTD |= 0x30;  // Displays desligados 

  // --------------------------------------


  // Configuração do Temporizador (no final do código)
  // bloco do fluxograma [4]
  configuracao_Timer0();

  // Configuração do Botão  (no final do código)
  // bloco do fluxograma [5]
  configuracao_int_botao();

}



// Laço
void loop(){

  // Atraso para melhor funcionamento da simulação no Tinkercad
  _delay_ms(1);

  // Verifico a luminosidade 
  // bloco do fluxograma [6]
  LDR_f();

  // caso esteja dia ou se já algum momento foi dia e o botão foi pressionado a condição abaixo sempre será verdade. 
  // Fiz isso para que caso fique de noite durante as ações geradas por ter pressionado o botão essas ações continuem até finalizarem
  // bloco do fluxograma [7]
  if (esta_dia || exec_da_parada){
    // Caso tenha sido registrado que o botão foi pressionado
    // bloco do fluxograma [8]
    if(status_bot){

      // variável para forçar que as ações do botão continuem até acabarem todas
      exec_da_parada = 1;

      //  Condição incial responsável por apagar as luzes e acender a amarela por um tempo
      // bloco do fluxograma [9]
      if(inicio_do_ped){
        // bloco do fluxograma [10]
        // Seto a variável para dar início no contador do tempo de espera da luz amarela, apenas quando o contador ainda não foi iniciado
        if(!cont_960) timer_amarelo = 1;  
        
        // após 960 ms 
        if(cont_960 > 120) {
          inicio_do_ped = 0, timer_amarelo = 0, prox_est = 1;  // Deixo a luz amarela 480 ms acesa
          // Permito que o pedestre passe e os veículos parem
          PORTC &= 0xEB;
          PORTC |= 0x22;
        }else if(cont_960 > 60){  // após 480 ms
          PORTC &= 0xF1;  // apago todas as luzes do semáforo dos veiculos
          PORTC |= 0x04;  // Acendo a luz amarela do semáforo dos veículos
        }

      }else{
        // bloco do fluxograma [11]
        // quando termino as ações iniciais habilito para começar a ligar os display, um de cada vez a cada 8 ms
        troca_disp = 1;

        // Nas condições seguintes faço a alternância entre qual display fica ativado
        if(sel_disp){ // display dos veículos
          // desabilitar PD5 e habilito PD4
          PORTD |= 0x10;
          PORTB = reg_disp_v;
          PORTD &= ~0x20;
        }else{  // display dos pedestres
          // desabilito PD4 e habilito PD5
          PORTD |= 0x20;
          PORTB = reg_disp_p;
          PORTD &= ~0x10;
        }

        // Quando acabar a máquina de estado reinicio as variáveis necessárias e finalizo as ações do botão
        // bloco do fluxograma [12]
        if(fim_maq){
          // bloco do fluxograma [14]
          exec_da_parada = 0;
          status_bot = 0;
          // Apago os displays
          PORTD |= 0x30;
          // Habilito verde para carros e vermelho para pedestres
          PORTC &= 0xC9;
          PORTC |= 0x18; 
          cont_disp = 0;
          inicio_do_ped = 1;
          cont_960 = 0;
          troca_disp = 0;
          fim_maq = 0;
        }else{
          // bloco do fluxograma [13]
          // enquanto não acabar a máquina de estado espero um tempo de ~0.5s para cada estado
          if(prox_est){
            maq_est();
            // zero estado para que não gere nenhum problema enquanto está fazendo as ações do estado atual
            prox_est = 0;
          }
        }
        
      }
    }else{
      // bloco do fluxograma [15]
      // caso o botão não esteja pressionado deixo verde para os carros e vermelho para os pedestres
      PORTC &= 0xC9;
      PORTC |= 0x18; 
    }

  }else{
    // Pisca (0.5 s) luz amarela(A2) para os carros e vermelha(A4) para os pedestres
    // faço a alternância entre ligado e desligado
    // bloco do fluxograma [16]
    if(pisca){ 
      pisca = 0;
      PORTC &= 0xD5;  
      // aqui verifico a variável para ver se deixo aceso ou se apago os leds vermelho e amarelo
      if(estado_pis){
        PORTC |= 0x14;
        estado_pis = 0;
      }else{
        PORTC &= ~0x14;
        estado_pis = 1;
      }
    }

  }

}

void LDR_f(){
  // Verifico a intensidade solar a cada 1120 ms


  if(timer_LDR){  // a cada 112 ms faço uma checagem
    timer_LDR = 0;  // zera a variável para que somente seja setada novamente após o tempo de 112 ms
    vet_ADC[aux_ADC] = analogRead(LDR); // guardo um dos registros no vetor
    aux_ADC++;
    // quando tiver 10 elementos registrados no vetor somo eles em uma nova variável
    if(aux_ADC > 9){ 
      aux_ADC = 0;
      for(char i = 0; i < 10; i++){
        val_ADC += vet_ADC[i];
      }
      val_ADC /= 10; // faço a média aritmética dos valores

      // Verifico se o valor digital é para o período de dia
      if(val_ADC < 200){          
        esta_dia = 1;
      }else{
        esta_dia = 0;
      }
    }
  }
}

void maq_est(){

    // máquina de estado com 20 estados -> um para cada 0.5 segundos

    switch(cont_maq){

    case 0:
      reg_disp_v = 0x09;
      reg_disp_p = 0x05;
      break;
    case 1:
      reg_disp_v = 0x09;
      reg_disp_p = 0x05;
      break;
    case 2:
      reg_disp_v = 0x08;
      reg_disp_p = 0x04;
      break;
    case 3:
      reg_disp_v = 0x08;
      reg_disp_p = 0x04;
      break;
    case 4:
      reg_disp_v = 0x07;
      reg_disp_p = 0x03;
      break;
    case 5:
      reg_disp_v = 0x07;
      reg_disp_p = 0x03;
      break;
    case 6:
      reg_disp_v = 0x06;
      reg_disp_p = 0x02;
      break;
    case 7:
      reg_disp_v = 0x06;
      reg_disp_p = 0x02;
      break;
    case 8:
      reg_disp_v = 0x05;
      reg_disp_p = 0x01;
      break;
    case 9:
      reg_disp_v = 0x05;
      reg_disp_p = 0x01;
      break;
    case 10:
      reg_disp_v = 0x04;
      reg_disp_p = 0x00;
      break;

      // a partir daqui começo a piscar o led vermelho pois o display dos pedestres atingiu o valor 0
    case 11:
      reg_disp_v = 0x04;
      reg_disp_p = 0x00;
      PORTC &= ~0x20;
      break;
    case 12:
      reg_disp_v = 0x03;
      reg_disp_p = 0xFF;
      PORTC &= ~0x10;
      break;
    case 13:
      reg_disp_v = 0x03;
      reg_disp_p = 0x00;
      PORTC |= 0x10;
      break;
    case 14:
      reg_disp_v = 0x02;
      reg_disp_p = 0xFF;
      PORTC &= ~0x10;
      break;
    case 15:
      reg_disp_v = 0x02;
      reg_disp_p = 0x00;
      PORTC |= 0x10;
      break;
    case 16:
      reg_disp_v = 0x01;
      reg_disp_p = 0xFF;
      PORTC &= ~0x10;
      break;
    case 17:
      reg_disp_v = 0x01;
      reg_disp_p = 0x00;
      PORTC |= 0x10;
      break;
    case 18:
      reg_disp_v = 0x00;
      reg_disp_p = 0xFF;
      PORTC &= ~0x10;
      break;
    case 19:
      reg_disp_v = 0x00;
      reg_disp_p = 0x00;
      PORTC |= 0x10;
      break;
  }
  cont_maq++;
  // deixo acontecer um estado a mais, no caso ele repete o último para que os dois diplays apaguem juntos no final
  if(cont_maq > 20) cont_maq = 0, fim_maq = 1;

}

void configuracao_Timer0(){
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Configuracao Temporizador 0 (8 bits) para gerar interrupcoes periodicas a cada 8ms no modo Clear Timer on Compare Match (CTC)
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

void configuracao_int_botao(){
  SREG |= 0x80;     // Ativo a interrupção global para ativar as interrupções
  PCICR = 0x04;     // Ativando a interrupção da fileira de PCINT23 a PCINT16
  PCMSK2 = 0x40;    // Ativo a interrupção apenas do PCINT22 que é o PD6
  // Bordas de subida e descida geram interrupções
}
  
  
