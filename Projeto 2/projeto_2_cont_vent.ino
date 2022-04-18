// Atividade Parcial 1 - Recepção e decodificação de comandos
// Gustavo Freitas Alves & Jitesh Ashok Manilal Vassaram

/*
 * Função que concatena as informações recebidas e retorna o comando --> Func_recebimento
    - Apenas checo o comando recebido e se há algum erro
 * Função que decodifica o comando e envia a resposta via Serial --> Func_deco
    - Verifico a velocidade e se há algum erro nesse parâmetro
*/

// -------------------------------------------------------------------------------

// Buffer para armazernar temporariamente os bytes lidos na serial  
byte bufferByte[8];     // maior entrada terá 8 bytes contando com o *

// Contador para identificar e manipular elementos no vetor buffer
int cont_vet = 0;

// Vetor para guardar mensagem recebida e futuramente ser entregue para o identificador
char mensagem[7];       // maior mensagem a ser armazenada terá 7 bytes

// Variável para armazenar tamanho da mensagem recebida
int tam_msg = 0;

// Variável para identificar que a mensagem foi devidamente recebida
bool msg_recebida = 0;

String vel = ""; // String que armazena o valor da velocidade do comando "VEL"

String codigo = "";


void setup() {
  Serial.begin(9600); 
}

void loop() {

  fun_deco();

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
      msg_recebida = 1;
      cont_vet = 0;
      return msg_recebida;

    }else{

      // Avanço o contador
      cont_vet++;
        
      // Reseto a posição de indicação do elemento do vetor -> Faço isso, pois, se caso algum usuário inserir 8 bytes sem nenhum *
      // e como não há nenhum comando com mais de 8 bytes
      if(cont_vet > 7) cont_vet = 0;
    }
  }
    
}

// Função que decodifica a mensagem que foi enviada ao monitor, e para o caso de setar a velocidade, retorna o valor da velocidade
long fun_deco() {
    
    fun_receber();
    
    // Verifica se a variavel de sinalizacao de mensagem foi setada
    if(msg_recebida) {
        
        msg_recebida = 0; // Zera a variavel de sinalizacao antes de fazer a decodificacao
        
        // Verifica qual comando foi escrito no monitor serial, para enviar a UART sua respectiva mensagem (de erro ou nao)
        if (codigo.equals("VENT")) {
            Serial.print("OK VENT");
          	Serial.println();
        }
        else if (codigo.equals("EXAUST")) {
            Serial.print("OK EXAUST");
          	Serial.println();
        }
        else if (codigo.equals("PARA")) {
            Serial.print("OK PARA");
          	Serial.println();
        }
        else if (codigo.equals("RETVEL")) {
            Serial.print("VEL: X RPM");
          	Serial.println();
        }
        
        /* Comando de velocidade - para esse comando, é feito a identificacao do comando "VEL" atraves dos 3 primeiros elementos da variavel 'codigo'
           e, após isso, é verificado se a o numero que seta a velocidade do motor esta no formato xxx (xxx entre 000 e 100) */
        else if (codigo.substring(0, 3).equals("VEL")) {

            codigo.remove(0,4); // Remove os 4 primeiros elementos do comando ('VEL '), para que seja trabalhado somente com os numeros

            /* Verifica se o numero do comando enviado está no formato correto ('xxx'), e para isso, utiliza-se o length() para ver se o tamanho do comando
               enviado corresponde com o tamanho do formato desejado */
            if (codigo.length() == 3) {
                
                vel = codigo;
                if (vel.toInt() >= 0 && vel.toInt() <= 100) {
                    Serial.print("OK VEL ");
                    Serial.print(vel);
                    Serial.print("%");
                    Serial.println();
                    
                    return vel.toInt();
                }
                else {
                    Serial.print("ERRO: PAR\302METRO INCORRETO");
                    Serial.println();
                }
            }
            /* Caso o tamanho do comando enviado não corresponda ao tamanho desejado, é enviado a mensagem de parametro incorreto */
            else if (codigo.length() < 3 && codigo.length() > 0) {
                Serial.print("ERRO: PAR\302METRO INCORRETO");
                Serial.println();
            }
            /* Caso o tamanho do numeros for 0, isto é, não foi dgitado nenhum numero, é enviado uma mensagem de parâmetro ausente*/
            else {
                Serial.print("ERRO: PAR\302METRO AUSENTE");
                Serial.println();
            }
        }

        /* Caso a mensagem enviada ao monitor serial não corresponda com nenhum dos casos anteriores ou que a mensagem extrapolou o tamanho 
           definido (tamanho maximo do buffer é de 8 - contando com o *) é enviado a mensagem de erro de comando inexistente */ 
        else {
            Serial.print("ERRO: COMANDO INEXISTENTE");
            Serial.println();
        }
    } 
}
