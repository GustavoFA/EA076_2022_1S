// Atividade Parcial 1 - Recepção e decodificação de comandos
// Gustavo Freitas Alves

/*
 * Função que concatena as informações recebidas e retorna o comando --> Func_recebimento
    - Apenas checo o comando recebido e se há algum erro
 * Função que decodifica o comando e envia a resposta via Serial --> Func_deco
    - Verifico a velocidade e se há algum erro nesse parâmetro
*/

// variavel para identificar se tem mensagem nova
bool mens_nova = 0;
// variável auxiliar para armazenar a string lida na Serial
String mensagem;

// string auxiliar para obter a velocidade
String vel_string;

// variavel para identificar o comando a ser executado
int comando = 0;

// Variável global para armazenamento do valor da velocidade
int vel = 0;

void setup(){

    Serial.begin(9600);

}

void loop(){
    
    //Func_recebimento();
    
}


int Func_recebimento(){

    // verifica se há dados chegando e caso tenha guardo-o
    if(Serial.available() > 0){
        mensagem = Serial.readString();
        mens_nova = 1;
    }
    
    if(mens_nova){
        if(mensagem == "EXAUST*"){
            comando = 1;
        }else if (mensagem == "PARA*"){
            comando = 2;
        }else if(mensagem == "RETVEL*"){
            comando = 3;
        }else if(mensagem[0] == 'V' && mensagem[1] == 'E' && mensagem[2] == 'L'){
            comando = 4;
        }else{
            comando = 0;
        }
        mens_nova = 0;
        mensagem = ""; 
    }

    return comando;
}

void Func_deco(int par){

    // começo pela primeiro elemento que representa o valor da velocidade
    for(int i = 4; i < 7; i++){
        vel_string += mensagem[i];
    }

    if(vel_string == "   ");

    // verifico se a velocidade está fora de escala
    if(vel_string.toInt() > 100) comando = 5;

    switch (par)
    {
    case 0:
        Serial.println("ERRO: COMANDO INEXISTENTE");
        break;
    case 1:
        Serial.println("OK EXAUST");
        break;
    case 2:
        Serial.println("OK PARA");
        break;
    case 3:
        Serial.print("OK ");
        // tem que obter a velocidade e printar aqui
        Serial.println(" RPM");
        break;
    case 4:
        Serial.print("OK VEL ");
        Serial.print(vel_string);
        Serial.println("%");
        break;
    }


}
