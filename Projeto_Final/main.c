#include <avr/io.h>
#include "serial_printf.h"

#define Sensor_OUT5 PC5
#define Sensor_OUT4 PC4
#define Sensor_OUT3 PC3
#define Sensor_OUT2 PC2
#define Sensor_OUT1 PC1

/*OC2B PD3
 * OC2A PB3 */
/*y=2,55X+255*/

uint8_t Sensor[4];

/*Fazer a inicialização das variaveis*/
void Init();

/*Muda o vetor Sensor de acordo com o input*/
void Sensores();

/*Modo debug*/
#ifdef DEBUG
void Debug_Printf();
#endif

int main(void) {
	Init();

	while (1) {
		Sensores();

#ifdef DEBUG
		Debug_Printf();
#endif
	}
}

void Init() {

	/*PWM Timer*/
	TCCR2B = 0; // Stop TC2
	TIFR2 |= (7<<TOV2); // Clear pending intr
	TCCR2A = (3<<WGM20)|(1<<COM2A0); // Fast PWM
	TCCR2B |= (3<<WGM02); // Set at TOP
	TCNT2 = 0; // Load BOTTOM value
	OCR2A = T2TOP; // Load TOP value
	TIMSK2 = 0; // Disable interrupts
	TCCR2B = 6; // Start TC2 (TP=256)


	printf_init();
	printf("Robot\n");
	DDRC &= ~((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));
	PORTC |= ((1 << Sensor_OUT5) | (1 << Sensor_OUT4) | (1 << Sensor_OUT3)
			| (1 << Sensor_OUT2) | (1 << Sensor_OUT1));
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

#ifdef DEBUG
void Debug_Printf() {
	printf("%d %d %d %d %d\n\n\n\n", Sensor[4], Sensor[3], Sensor[2], Sensor[1],
			Sensor[0]);
}
#endif
