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


void setup() {
  Serial.begin(9600); 
}

void loop() {

    
  // Função usada para checar se a fun_receber estava ok
  /*
  if(fun_receber()){
    msg_recebida = 0;
    for(int j=0; j<tam_msg;j++){
      Serial.print(mensagem[j]);
    }
    Serial.println();
  }
  */

}

// Função que retornar se a mensagem já foi recebida
// Essa função não identifica uma mensagem sem * 
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
