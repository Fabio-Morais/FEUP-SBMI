//#define DEBUG
#define BLUE
/*Bibliotecas padrao*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*Ficheiros de bilbiotecas*/
#ifdef DEBUG
#include "serial_printf.h"
#endif

#ifndef DEBUG
#include "Led.h"
#include "Bluetooth.h"
#endif

/*SENSORES*/
#define Sensor_OUT5 PC5
#define Sensor_OUT4 PC4
#define Sensor_OUT3 PC3
#define Sensor_OUT2 PC2
#define Sensor_OUT1 PC1

/*MOTORES*/
#define Motor_E PB3
#define Motor_D PD3

/*LEDS*/
#define LED_AZUL PC0
#define LED_VERMELHO PB0

/*VALIDAÇÃO*/
#define OK -1
#define ESQUERDA_ESQUERDA -2
#define ESQUERDA -3
#define DIREITA_DIREITA -4
#define DIREITA -5
#define PARADO -6

/* 0-> Parado
 * 255-> Velocidade máxima*/
/*VELOCIDADES PADRÃO*/
#define Velocidade_Padrao 230
#define Mudanca_Suave_Padrao 10
#define Mudanca_Bruta_Padrao 160

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
uint8_t Velocidade_Default, Mudanca_Suave, Mudanca_Bruta; //Velocidades para manipular
volatile uint16_t Tempo_1ms, Tempo_3s; //Tempos
volatile uint8_t Comando; //Start and Stop
volatile uint8_t Modo_Robo; // Modo manual ou automatico
volatile int8_t Controlo_Manual; // Variavel que armazena as direções em modo MANUAL
volatile uint8_t Flag_Ciclo; //Para correr no primeiro ciclo
/********************************************************************************/

/*Fazer a inicialização das variaveis*/
void Init();

/*Muda o vetor Sensor de acordo com o input*/
void Sensores();

/*Faz movimento dos motores de acordo com o estado dos sensores*/
void Calculo();

/*Envia para os motores os valores que a função Calculo fez*/
void Motores(int8_t Valid);

/* Coloca na variavei Velocidade a escolha do bluetooth
 * Recebe valores entre 1 e 5
 * 3 é o valor padrão
 * */
void Motor_Calculation(uint8_t Data);

/*Modo debug*/
#ifdef DEBUG
void Debug_Printf();
#endif

/********************************************************************************/

/* Interrupção do timer 1
 * Incrementa a cada 1ms */
ISR(TIMER1_OVF_vect) {
	TCNT1 = T1BOTTOM; // reload TC1
	if(Tempo_1ms>0)
		Tempo_1ms--;
	if(Tempo_3s>0)
		Tempo_3s--;
}

#ifdef BLUE
/*RECEBE DADOS BLUETOOTH*/
ISR(USART_RX_vect) {

	unsigned char RecByte;
	RecByte = (uint8_t) UDR0;
	if (RecByte == 151 || RecByte == 150)
		Comando = RecByte;
	else if ((RecByte >= 1) && (RecByte <= 3)) {
		if (RecByte == 1) //RECEBE DIREITA
			Controlo_Manual = DIREITA_DIREITA;
		else if (RecByte == 2) //RECEBE ESQUERDA
			Controlo_Manual = ESQUERDA_ESQUERDA;
		else if (RecByte == 3) //RECEBE FRENTE
				{
			Controlo_Manual = OK;
				}

	} else if (RecByte == 5 || (RecByte >= 11 && RecByte <= 14)) //RECEBE PARADO
		Controlo_Manual = PARADO;
	else if (RecByte == 40 || RecByte == 41)
		Modo_Robo = RecByte;

	Tempo_3s=2000;
}
#endif

/*********************************************************************************/

int main(void) {

uint8_t Controlo_Manual_X;

	Init();
#ifndef DEBUG
#ifdef BLUE
	init_usart();
#endif
#endif

	lcd_init();

	Comando = STOP;
	Modo_Robo = MODO_AUTOMATICO;
	Flag_Ciclo=0;

	while (1) {
		/*******************/
		/*ANDA COM SENSORES*/
		/*******************/
		if (Modo_Robo == MODO_AUTOMATICO) {
			Controlo_Manual = -111;
			Sensores();
			if (Comando == RUN) {
				Calculo();
				PORTC |= (1 << LED_AZUL);
				PORTB &= ~(1 << LED_VERMELHO);
				lcd_gotoxy(1, 2);
				lcd_print("RUN ");

			} else if (Comando == STOP) {
				Motores(PARADO);
				PORTB ^= (1 << LED_VERMELHO);
				PORTC &= ~(1 << LED_AZUL);
				lcd_gotoxy(1, 2);
				lcd_print("STOP");
			}
			lcd_print_lcd(Sensor);
		}
		/***************************/
		/*ANDA POR CONTROLO REMOTO*/
	    /**************************/
		else if (Modo_Robo == MODO_MANUAL) {
			/*Faz 1 vez no inicio*/
			if(!Flag_Ciclo)
			{
			lcdCommand(0x01);
			_delay_ms(20);
			lcd_gotoxy(1, 1);
			lcd_print("CONECTADO");
			Flag_Ciclo=1;
			PORTC |= (1 << LED_AZUL);
			PORTB &= ~(1 << LED_VERMELHO);
			}


			if (Controlo_Manual != -111) {
				Controlo_Manual_X=Controlo_Manual;
				Motores(Controlo_Manual_X);
				_delay_us(150);

			}

			/*Caso bluetooth perca comunicação */
			if(!Tempo_3s)
			{
				lcdCommand(0x01);
				lcd_gotoxy(1, 1);
				lcd_print("DESCONECTADO");
				_delay_ms(30);
				while(1)
				{
					PORTC ^= (1<<LED_AZUL);
					_delay_ms(400);
					Motores(PARADO);
				}
			}
		}

#ifndef DEBUG
#ifdef BLUE
		Send_Sensores(Sensor);
#endif
#endif

#ifdef DEBUG
		Debug_Printf();
		_delay_ms(1000);
#endif

	}
}

/********************************************************************************/

void Motor_Calculation(unsigned char Data) {

}

void Init() {

	/*PWM Timer*/
	TCCR2B = 0; // Stop TC2
	TIFR2 |= (7 << TOV2); // Clear pending intr
	TCCR2A = (3 << WGM20) | (1 << COM2A1) | (1 << COM2B1); // Fast PWM
	TCCR2B |= (1 << WGM22); // Set at TOP
	TCNT2 = 0; // Load BOTTOM value
	OCR2A = 127;
	OCR2B = 127;
	TIMSK2 = 0; // Disable interrupts
	TCCR2B = 2;

#ifdef BLUE
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
#endif

#ifdef DEBUG
	printf_init();
	printf("Robot\n");
#endif

	/*SENSORES COM PULL UP*/
	DDRC &= ~((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));

	PORTC |= ((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));

	/*LEDS*/
	DDRC |= (1 << LED_AZUL);
	DDRB |= (1 << LED_VERMELHO);

	/*MOTORES*/
	DDRB |= (1 << Motor_E);
	//DDRD = (1 << Motor_D);
	DDRD = 0xFF;

	PORTB = (1 << Motor_E);
	PORTD = (1 << Motor_D);

	/*ATRIBUIÇÃO DAS VELOCIDADES*/
	Velocidade_Default = Velocidade_Padrao;
	Mudanca_Bruta = Mudanca_Bruta_Padrao;
	Mudanca_Suave = Mudanca_Suave_Padrao;
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

	if (!Sensor[2])
		Motores(OK);
	else if (!Sensor[1])
		Motores(ESQUERDA);
	else if (!Sensor[0])
		Motores(ESQUERDA_ESQUERDA);
	else if (!Sensor[3])
		Motores(DIREITA);
	else if (!Sensor[4])
		Motores(DIREITA_DIREITA);

}
/*OCR2A -> MOTOR ESQUERDA
 * OCR2B-> MOTOR DIREITA*/
void Motores(int8_t Valid) {

	switch (Valid) {

	case OK:
		OCR2A = Velocidade_Default;
		OCR2B = Velocidade_Default;
		break;

	case ESQUERDA_ESQUERDA:
		OCR2A = Velocidade_Default - Mudanca_Bruta;
		OCR2B = Velocidade_Default;
		break;

	case ESQUERDA:
		OCR2A = Velocidade_Default - Mudanca_Suave;
		OCR2B = Velocidade_Default;
		break;

	case DIREITA_DIREITA:
		OCR2A = Velocidade_Default;
		OCR2B = Velocidade_Default - Mudanca_Bruta;
		break;

	case DIREITA:
		OCR2A = Velocidade_Default;
		OCR2B = Velocidade_Default - Mudanca_Suave;
		break;

	case PARADO:
		OCR2A = 0;
		OCR2B = 0;
		break;
	}

}

#ifdef DEBUG
void Debug_Printf() {
	printf("%d %d %d %d %d\nocra:%d  ocrB:%d->%d\n\n\n", Sensor[0], Sensor[1], Sensor[2], Sensor[3],Sensor[4], OCR2A, OCR2B, flag);
}
#endif
