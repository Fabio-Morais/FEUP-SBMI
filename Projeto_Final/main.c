/*******************************************
 *  main.c
 *  Created on: 24/11/2018 (eclipse, avr-gcc)
 *      Author: up201504257@fe.up.pt
 *      		up201708979@fe.up.pt
 *
 *	Objetivo: Implementar um robo segue linha, que comunica por bluetooth com uma app que desenvolvemos
 *	onde envia informação relativa aos sensores, nivel de bateria, se está "perdido" da pista ou não
 *	podemos mudar a velocidade, entre diversas outras informações.
 *	É possivel tambem com a nossa app mudar para o Modo Manual, onde podemos controlar o robo.
 *	Para alem disso, temos um comando IR que podemos dar o START e STOP, como mudar a sua velocidade
 *
 *
 *	Solução:
 *******************************************/

/*Bibliotecas padrao*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*Ficheiros de bilbiotecas*/
#include "Lcd.h"
#include "Bluetooth.h"
#include "Battery.h"

/*SENSORES*/
#define Sensor_OUT5 PC5
#define Sensor_OUT4 PC4
#define Sensor_OUT3 PC3
#define Sensor_OUT2 PC2
#define Sensor_OUT1 PC1

/*MOTORES*/
#define Motor_E PB3
#define Motor_D PD3
#define Motor_E_T PB2
#define Motor_D_T PB1

/*LEDS*/
#define LED_AZUL PC0
#define LED_VERMELHO PB0

/*VALIDAÇÃO*/
#define OK 20
/*Esquerda*/
#define ESQUERDA_BRUTA 21
#define ESQUERDA_SUAVE 22
#define ESQUERDA_MEDIA 23
#define ESQUERDA_MEDIA_MAIS 24
/*Direita*/
#define DIREITA_BRUTA 25
#define DIREITA_MEDIA 26
#define DIREITA_MEDIA_MAIS 27
#define DIREITA_SUAVE 28
/*Especiais*/
#define PARADO 29 //Trava
#define REVERSE 30 //Marcha atrás

/* 0-> Parado
 * 255-> Velocidade máxima*/
/*VELOCIDADES PADRÃO*/
#define Velocidade_Padrao 220
#define Mudanca_Suave_Padrao 15
#define Mudanca_Media_Padrao 30
#define Mudanca_Media_Mais_Padrao 60
#define Mudanca_Bruta_Padrao 200

/*TIMERS*/
#define T1BOTTOM 65536-16000
#define T2TOP 255

/*MODO DE OPERAÇÃO*/
#define MODO_MANUAL 40
#define MODO_AUTOMATICO 41

/*ESTADO ROBO*/
#define RUN 150
#define STOP 151

/*VARIAVEIS*/
uint8_t Sensor[5];	//Sensor Linha
volatile uint8_t Velocidade_Default, Mudanca_Suave, Mudanca_Media,
		Mudanca_Media_mais, Mudanca_Bruta; //Velocidades para manipular
volatile uint16_t Tempo_1ms, Tempo_3s, Tempo_Perdido, Tempo_Led; //Tempos
volatile uint8_t Comando; //Start and Stop
volatile uint8_t Modo_Robo; // Modo manual ou automatico
volatile int8_t Controlo_Manual; // Variavel que armazena as direções em modo MANUAL
volatile uint8_t Flag_Ciclo; //Para correr no primeiro ciclo
uint8_t Robo_Perdido; //Flag que diz se robo está perdido ou não
uint8_t Volta; //Numero da volta
uint8_t aux;//flag do rising edge para contar o numero de voltas


/********************************************************************************/

/*Fazer a inicialização das variaveis*/
void Init();

/*Muda o vetor Sensor de acordo com o input*/
void Sensores();

/*Faz movimento dos motores de acordo com o estado dos sensores*/
void Calculo();

/*Envia para os motores os valores que a função Calculo fez*/
void Motores(uint8_t Valid);


/*Imprime lcd | Acende luz azul*/
void Modo_Run(void);

/*Imprime lcd | Pisca luz vermelha*/
void Modo_Stop(void);

/*Para motores | Pisca luz vermelha | imprime lcd*/
void Modo_Perdido(void);

/*Acende luz azul | Imprime Lcd*/
void Incializa_Manual(void);

/*Conta volta e envia para bluetooth e imprime no lcd*/
void Conta_Volta(void);


/********************************************************************************/

/* Interrupção do timer 1
 * Incrementa a cada 1ms */
ISR(TIMER1_OVF_vect) {
	TCNT1 = T1BOTTOM; // reload TC1
	if (Tempo_1ms > 0)
		Tempo_1ms--;
	if (Tempo_3s > 0)
		Tempo_3s--;
	if (Tempo_Perdido > 0)
		Tempo_Perdido--;
	if(Tempo_Led>0)
		Tempo_Led--;
}

/*RECEBE DADOS BLUETOOTH*/
ISR(USART_RX_vect) {

	unsigned char RecByte;
	RecByte = (unsigned char) UDR0;

	/*******************************************/
	/*CONTROLO AUTOMATICO*/

	/*START AND STOP*/
	if (RecByte == 151 || RecByte == 150)
		Comando = RecByte;
	/*Recebe valores de velocidade a escolher*/
	else if (RecByte >= 61 && RecByte <= 65) {
		if (RecByte == 61)
			Velocidade_Default = 160;
		else if (RecByte == 62)
			Velocidade_Default = 180;
		else if (RecByte == 63)
			Velocidade_Default = 200;
		else if (RecByte == 64)
			Velocidade_Default = 220;
		else if (RecByte == 65)
			Velocidade_Default = 240;
	}
	/*******************************************/
	/*CONTROLO MANUAL*/
	else if ((RecByte >= 1) && (RecByte <= 4)) {
		if (RecByte == 1) //RECEBE DIREITA
			Controlo_Manual = DIREITA_BRUTA;
		else if (RecByte == 2) //RECEBE ESQUERDA
			Controlo_Manual = ESQUERDA_BRUTA;
		else if (RecByte == 3) //RECEBE FRENTE
			Controlo_Manual = OK;
		else if(RecByte == 4)
			Controlo_Manual = REVERSE;
	} else if (RecByte == 5 || (RecByte >= 11 && RecByte <= 14)) //RECEBE PARADO
		Controlo_Manual = PARADO;
	else if (RecByte == 40 || RecByte == 41)
		Modo_Robo = RecByte;

	/*Recebe valores de velocidade a escolher*/
	else if (RecByte >= 51 && RecByte <= 55) {
		if (RecByte == 51)
			Velocidade_Default = 160;
		else if (RecByte == 52)
			Velocidade_Default = 180;
		else if (RecByte == 53)
			Velocidade_Default = 200;
		else if (RecByte == 54)
			Velocidade_Default = 220;
		else if (RecByte == 55)
			Velocidade_Default = 240;
	}

	/*Para confirmar se continua a receber informaçao via bluetooth*/
	Tempo_3s = 2000;
}


/*********************************************************************************/

int main(void) {

	Init();


	init_usart();

	stdout = &mystdout;
	lcd_init();

	/*Inicia variaveis*/
	Comando = RUN;
	Modo_Robo = MODO_AUTOMATICO;
	Flag_Ciclo = 0;
	Robo_Perdido = 0;

	while (1) {
		Send_Data(122);
		/*********************************************/
		/*ANDA COM SENSORES*/
		/**********************************************/
		if (Modo_Robo == MODO_AUTOMATICO) {
			Controlo_Manual = -111;

			Sensores(); // Coloca na varivel Sensores os valores ativos

			if (Comando == RUN) { //Robo em andamento

				Calculo(); // Toma açoes nos motores de acordo com sensores

				if (Robo_Perdido) { //Não está na pista

					_delay_ms(100);

					Motores(REVERSE);

					if (!Tempo_Perdido) { //Está perdido | Fica parado
						uint8_t Flag_Perdido=0;
						while (1) {
							if(!Flag_Perdido)
							{
								Modo_Perdido();
								Flag_Perdido=1;
								Tempo_Led=100; //150ms de tempo
								Send_Data(122); // envia que ficou perdido
								_delay_ms(2);
							}
							if(!Tempo_Led){
								PORTB ^= (1 << LED_VERMELHO);
								Tempo_Led=100;
							}
							Sensores(); // Coloca na varivel Sensores os valores ativos
							if (!Sensor[0] || !Sensor[1] || !Sensor[2] || !Sensor[3] || !Sensor[4]) {
								Robo_Perdido = 0;
								lcd_init();
								Send_Data(123); // envia que saiu de Perdido
								_delay_ms(2);
								break;
							}
						}
					}
				} else { // Está tudo OK

					Modo_Run();
				}

			} else if (Comando == STOP) { // Robo parado
				Modo_Stop();
			}

			lcd_print_lcd(Sensor); //imprime informaçao dos sensores
			Conta_Volta(); //Conta numero de voltas e imprime

			Send_Sensores(Sensor); // Envia Sensores via bluetooth

		}
		/************************************************************/
		/*ANDA POR CONTROLO REMOTO*/
		/**************************************************************/
		else if (Modo_Robo == MODO_MANUAL) {

			/*Faz 1 vez no inicio*/
			if (!Flag_Ciclo) {
				Incializa_Manual(); // Imprime lcd | Acende luz
			}

			if (Controlo_Manual != -111) {
				Motores(Controlo_Manual);
				_delay_us(150);

			}

			/*Caso bluetooth perca comunicação */
			if (!Tempo_3s) {
				lcdCommand(0x01);
				lcd_gotoxy(1, 1);
				lcd_print("DESCONECTADO");
				_delay_ms(30);
				while (1) {
					PORTC ^= (1 << LED_AZUL);
					_delay_ms(400);
					Motores(PARADO);
					if(Tempo_3s) //Caso receba algum valor via bluetooth, o tempo dá reset
						break;
				}
			}

		}



	}
}

/********************************************************************************/


void Init() {

	/*PWM para motor frente*/
	TCCR2B = 0; // Stop TC2
	TIFR2 |= (7 << TOV2); // Clear pending intr
	TCCR2A = (3 << WGM20) | (1 << COM2A1) | (1 << COM2B1); // Fast PWM
	TCCR2B |= (1 << WGM22); // Set at TOP
	TCNT2 = 0; // Load BOTTOM value
	OCR2A = 127;
	OCR2B = 127;
	TIMSK2 = 0; // Disable interrupts
	TCCR2B = 2;

	/*Timer 1*/
	TCCR1B = 0; // Stop TC1
	TIFR1 = (7 << TOV1) // Clear all
	| (1 << ICF1); // pending interrupts
	TCCR1A = 0; // NORMAL mode
	TCNT1 = T1BOTTOM; // Load BOTTOM value
	TIMSK1 = (1 << TOIE1); // Enable Ovf intrpt
	TCCR1B = 1; // Start TC1
	/*1->1
	 * 2->8
	 * 3->64*/
	/*enable interrupt.*/
	sei();




	/*SENSORES COM PULL UP*/
	DDRC &= ~((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));

	PORTC |= ((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));

	/*LEDS*/
	DDRC |= (1 << LED_AZUL);
	DDRB |= (1 << LED_VERMELHO);

	/*MOTORES*/
	DDRB |= (1 << Motor_E) | (1 << Motor_E_T) | (1 << Motor_D_T);
	DDRD = 0xFF;
	PORTB |= (1 << Motor_E);
	PORTD |= (1 << Motor_D);

	/*ATRIBUIÇÃO DAS VELOCIDADES*/
	Velocidade_Default = Velocidade_Padrao;
	Mudanca_Bruta = Mudanca_Bruta_Padrao;
	Mudanca_Suave = Mudanca_Suave_Padrao;
	Mudanca_Media = Mudanca_Media_Padrao;
	Mudanca_Media_mais = Mudanca_Media_Mais_Padrao;

	/*Colocar numero de voltas a 1*/
	Volta=1;
}

/* [ OUT1  OUT2  OUT3  OUT4  OUT5 ]
 *    0     1     2      3     4
 * */
void Sensores() {
	if (PINC & (1 << Sensor_OUT5))
		Sensor[4] = 1;
	else
		Sensor[4] = 0;

	if (PINC & (1 << Sensor_OUT4))
		Sensor[3] = 1;
	else
		Sensor[3] = 0;

	if (PINC & (1 << Sensor_OUT3))
		Sensor[2] = 1;
	else
		Sensor[2] = 0;

	if (PINC & (1 << Sensor_OUT2))
		Sensor[1] = 1;
	else
		Sensor[1] = 0;

	if (PINC & (1 << Sensor_OUT1))
		Sensor[0] = 1;
	else
		Sensor[0] = 0;
}

/* [ OUT1  OUT2  OUT3  OUT4  OUT5 ]
 *    0     1     2      3     4
 * */

void Calculo() {

	if(Robo_Perdido && (!Sensor[0] || !Sensor[1] || !Sensor[2] || !Sensor[3] || !Sensor[4]))
	{
		Motores(PARADO);
		_delay_ms(10);
		Robo_Perdido=0;
	}

	if (Sensor[0] && Sensor[1] && Sensor[2] && Sensor[3] && Sensor[4]
			&& !Robo_Perdido) {
		Tempo_Perdido = 2000;
		Robo_Perdido = 1;
		Motores(PARADO);

	/*00100*/
	} else if (!Sensor[2] && Sensor[0] && Sensor[1] && Sensor[3] && Sensor[4]) {
		Motores(OK);
		Robo_Perdido = 0;
	}
	/*01000*/
	else if (!Sensor[1] && Sensor[0] && Sensor[2] && Sensor[3] && Sensor[4] ) {
		Motores(ESQUERDA_SUAVE);
		Robo_Perdido = 0;
	}
	/*10000*/
	else if (!Sensor[0] && Sensor[1] && Sensor[2] && Sensor[3] && Sensor[4]) {
		Motores(ESQUERDA_BRUTA);
		Robo_Perdido = 0;
	}
	/*00010*/
	else if (!Sensor[3] && Sensor[0] && Sensor[1] && Sensor[2] && Sensor[4]) {
		Motores(DIREITA_SUAVE);
		Robo_Perdido = 0;
	}
	/*00001*/
	else if (!Sensor[4] && Sensor[0] && Sensor[1] && Sensor[2] && Sensor[3]) {
		Motores(DIREITA_BRUTA);
		Robo_Perdido = 0;
	}


}
/*OCR2A -> MOTOR ESQUERDA
 * OCR2B-> MOTOR DIREITA*/
void Motores(uint8_t Valid) {

	switch (Valid) {

	case OK:
		PORTB &= ~((1 << Motor_E_T) | (1 << Motor_D_T));
		OCR2A = Velocidade_Default;
		OCR2B = Velocidade_Default;
		break;

	case ESQUERDA_BRUTA:
		PORTB &= ~((1 << Motor_E_T) | (1 << Motor_D_T));
		OCR2A = Velocidade_Default - Mudanca_Bruta;
		OCR2B = Velocidade_Default;
		break;

	case ESQUERDA_SUAVE:
		PORTB &= ~((1 << Motor_E_T) | (1 << Motor_D_T));
		OCR2A = Velocidade_Default - Mudanca_Suave;
		OCR2B = Velocidade_Default;
		break;

	case DIREITA_BRUTA:
		PORTB &= ~((1 << Motor_E_T) | (1 << Motor_D_T));
		OCR2A = Velocidade_Default;
		OCR2B = Velocidade_Default - Mudanca_Bruta;
		break;

	case DIREITA_SUAVE:
		PORTB &= ~((1 << Motor_E_T) | (1 << Motor_D_T));
		OCR2A = Velocidade_Default;
		OCR2B = Velocidade_Default - Mudanca_Suave;
		break;

	case REVERSE:
		OCR2A = 0;
		OCR2B = 0;
		PORTB |= (1 << Motor_E_T) | (1 << Motor_D_T);

		break;

	case PARADO:
		OCR2A = 255;
		OCR2B = 255;
		PORTB |= (1 << Motor_E_T) | (1 << Motor_D_T);
		break;

	default:
		PORTB &= ~((1 << Motor_E_T) | (1 << Motor_D_T));
		OCR2A = 0;
		OCR2B = 0;
		break;
	}

}

void Modo_Run(void) {

	PORTC |= (1 << LED_AZUL);
	PORTB &= ~(1 << LED_VERMELHO);
	lcd_gotoxy(1, 2);
	printf("RUN");
	Send_Data(125); //Envia que está em run
	_delay_ms(2);
}

void Modo_Stop(void) {

	Motores(PARADO);
	PORTB ^= (1 << LED_VERMELHO);
	PORTC &= ~(1 << LED_AZUL);
	lcd_gotoxy(1, 2);
	printf("STOP");
	Send_Data(126); //Envia que está em STOP
	_delay_ms(2);
}

void Modo_Perdido(void) {

	Motores(PARADO);
	PORTC &= ~(1 << LED_AZUL);
	lcdCommand(0x01);
	lcd_gotoxy(1, 1);
	printf("ROBO PERDIDO");
	lcd_gotoxy(1, 2);
	printf("Coloque na pista");
}

void Incializa_Manual(void) {

	lcdCommand(0x01);
	_delay_ms(20);
	lcd_gotoxy(1, 1);
	printf("CONECTADO");
	Flag_Ciclo = 1;
	PORTC |= (1 << LED_AZUL);
	PORTB &= ~(1 << LED_VERMELHO);
}

void Conta_Volta(void) {


	/*Conta a volta */
	if (!Sensor[0] && !Sensor[1] && !Sensor[2] && !Sensor[3] && !Sensor[4])
		aux = 1;
	else if ((aux == 1) && (Sensor[0] || Sensor[4]))
		aux = 2;
	else if (aux == 2) {
		Send_Data(70);
		aux = 0;
		_delay_ms(3);
		Volta++;
	}

	lcd_gotoxy(6, 2);
	printf("Volta: %d", Volta);



}

