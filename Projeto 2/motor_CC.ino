// Atividade Parcial 1 - Recepção e decodificação de comandos
// Gustavo Freitas Alves & Jitesh Ashok Manilal Vassaram


// -------------------------------------------------------------------------------

// Módulo Driver L293D

/*
  L293D
    Pinagem:
        - Inputs: 1A(2), 2A(7) e 1,2EN(1)
        - Outputs: 1Y(3) e 2Y(6)
    PWM: utilizar no 1,2EN 

*/


// Temporizador para PWM do motor
void conf_PWM(){

    // 0 0 1 0 - - 0 1
    // OC2B em nível alto na subida e nível baixo na descida
    // PWM, Phase correct
    TCCR2A |= 0x21;
    TCCR2A &= ~0xC2;

    // 0 0 - - 1 1 0
    // Prescaler = 256
    TCCR2B |= 0x06;
    TCCR2B &= ~0xC0;

    // Registrador para comparação
    OCR2B = 0; // motor começa com duty cycle = 0 %

}



void setup(){


    // Configuração das GPIOs PD
    DDRD |= 0x1C;   // Saídas PD2, PD3 e PD4
    PORTD &= ~0x14; // Início em nível baixo para deixar o motor parado


    // Configuração do PWM
    conf_PWM();



}



void loop(){

    // Insira algum valor em OCR2B para modificar o PWM e o nível de PD2 e PD4 para selecionar movimento

}
