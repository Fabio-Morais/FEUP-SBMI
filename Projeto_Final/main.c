#include <avr/io.h>
#include <util/delay.h>

//#include "serial_printf.h"
#include "Led.h"
#include "Bluetooth.h"

//#define DEBUG
#define Sensor_OUT5 PC5
#define Sensor_OUT4 PC4
#define Sensor_OUT3 PC3
#define Sensor_OUT2 PC2
#define Sensor_OUT1 PC1

#define Motor_E PB3
#define Motor_D PD3

#define T2TOP 255

#define OK 1
#define ESQUERDA_ESQUERDA 2
#define ESQUERDA 3
#define DIREITA_DIREITA 4
#define DIREITA 5
#define ERRO 6

/* 0-> Paradp
 * 255-> Velocidade máxima*/
#define Velocidade_Default 200
#define Mudanca_Suave 50
#define Mudanca_Bruta 150




/*OC2B PD3
 * OC2A PB3 */
/*y= -2,55X+255*/

uint8_t Sensor[5];

/*Fazer a inicialização das variaveis*/
void Init();

/*Muda o vetor Sensor de acordo com o input*/
void Sensores();

void Calculo();

void Motores();

/*Modo debug*/
#ifdef DEBUG
void Debug_Printf();
#endif

int main(void) {
	Init();
	init_usart();
	while (1) {
		Sensores();
		Calculo();
		while ((UCSR0A & (1 << UDRE0)) == 0);
		 UDR0 = (unsigned char)'2';
		 _delay_ms(1000);


#ifdef DEBUG
		Debug_Printf();
#endif
	}
}

void Init() {

	/*PWM Timer*/
	TCCR2B = 0; // Stop TC2
	TIFR2 |= (7<<TOV2); // Clear pending intr
	TCCR2A = (3<<WGM20)|(1<<COM2A1) |(1<<COM2B1); // Fast PWM
	TCCR2B |= (1<<WGM22) ; // Set at TOP
	TCNT2 = 0; // Load BOTTOM value
	OCR2A=127;
	OCR2B=127;
	TIMSK2 = 0; // Disable interrupts
	TCCR2B = 6;

#ifdef DEBUG
	printf_init();
	printf("Robot\n");
#endif
	DDRC &= ~((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));
	PORTC |= ((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));
	DDRB = (1<<Motor_E);
	DDRD= (1<<Motor_D);
	PORTB = (1<<Motor_E);
	PORTD= (1<<Motor_D);

}

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

	if(!Sensor[2])
		Motores(OK);
	else if(!Sensor[1])
		Motores(ESQUERDA);
	else if(!Sensor[0])
		Motores(ESQUERDA_ESQUERDA);
	else if(!Sensor[3])
		Motores(DIREITA);
	else if(!Sensor[4])
		Motores(DIREITA_DIREITA);
	else
		Motores(ERRO);

}
/*OCR2A -> MOTOR ESQUERDA
 * OCR2B-> MOTOR DIREITA*/
void Motores(int Percentagem_Duty){

	switch(Percentagem_Duty){

	case OK:
		OCR2A=Velocidade_Default;
		OCR2B=Velocidade_Default;
		break;

	case ESQUERDA_ESQUERDA:
		OCR2A=Velocidade_Default-Mudanca_Bruta;
		OCR2B=Velocidade_Default;
		break;

	case ESQUERDA:
		OCR2A=Velocidade_Default-Mudanca_Suave;
		OCR2B=Velocidade_Default;
		break;

	case DIREITA_DIREITA:
		OCR2A=Velocidade_Default;
		OCR2B=Velocidade_Default-Mudanca_Bruta;
		break;

	case DIREITA:
		OCR2A=Velocidade_Default;
		OCR2B=Velocidade_Default-Mudanca_Suave;
		break;

	case ERRO:
		OCR2A=0;
		OCR2B=0;
		break;
	}

}

#ifdef DEBUG
void Debug_Printf() {
	printf("%d %d %d %d %d\nocra:%d  ocrB:%d\n\n\n", Sensor[0], Sensor[1], Sensor[2], Sensor[3],Sensor[4], OCR2A, OCR2B);
}
#endif
