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

    Serial.begin(9600);

    // Configuração das GPIOs PD
    DDRD |= 0x1C;   // Saídas PD2, PD3 e PD4
    PORTD &= ~0x14; // Início em nível baixo para deixar o motor parado


    // Configuração do PWM
    conf_PWM();



}

// Função para inserir tipo de movimento e tensão média (PWM)
void motor(char mov, int pwm){

    switch (mov)
    {
    // Paro o movimento dos motores
    PORTD &= ~0x14;
    case 'p':
        // Mantenho parado
        break;
    
    case 'a':
        // Movimento horário
        PORTD |= 0x04;
        break;

    case 'h':
        // Movimento anti-horário
        PORTD |= 0x10;
        break;
    }

    // Faixa de comparação: 0 --> 255
    // PWM: 0 --> 100 %
    // 1% = 25.5 
    OCR2B = (int) pwm*(25.5);

}

void loop(){

    // Teste do funcionamento
    motor('a', 50);

}
