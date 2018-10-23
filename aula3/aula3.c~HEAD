/*******************************************
 *  aula.c
 *  Created on: 06/10/2018 (eclipse, avr-gcc)
 *      Author: up201504257@fe.up.pt
 *
 *      Estado 1-> LUZ VERDE NORTE
 *      Estado 2-> LUZ AMARELA NORTE
 *      Estado 3-> LUZ VERMELHA NORTE
 *      Estado 4-> LUZ VERDE SUL
 *      Estado 5-> LUZ AMARELA SUL
 *      Estado 6-> LUZ VERMELHA SUL
 *
 *******************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define RED_E PB5
#define YELLOW_E PB4
#define GREEN_E PB3

#define RED_N PB2
#define YELLOW_N PB1
#define GREEN_N PB0

#define EMERGENCIA PD3

#define T1BOTTOM 65536-625
#define VERDE 1
#define AMARELO 2
#define VERMELHO 3

/*Tempos em ms*/
#define TEMPO_VERDE 2000
#define TEMPO_AMARELO 2000
#define TEMPO_VERMELHO 2000


uint8_t estado;
volatile uint8_t flag;
volatile int16_t T_Verde, T_Amarelo, T_Vermelho, T_Emergencia;

/*Testa funcionamento de todos os LEDS */
void Testa_Leds(void);

/* Iniciação das variaveis e timers */
void hw_init(void);

/* Transições entre estados*/
void Estados(void);

/* Pisca o LED amarelo, posição irregular*/
void Erro(void);

/* Ativa as saidas (LEDS) de acordo com os estados*/
void Ativa_Saidas(void);

/*Dá START no timer*/
void Start_Time(uint16_t Time_ms, uint8_t escolha);

/*Faz inicialiazação do Timer 1*/
void Init_Time();

/*Interrupção*/
ISR(INT1_vect) {

	flag = 1;
}

ISR(TIMER1_OVF_vect) {

	TCNT1 = T1BOTTOM;

	if (T_Verde > 0)
		T_Verde--;

	if (T_Amarelo > 0)
		T_Amarelo--;

	if (T_Vermelho > 0)
		T_Vermelho--;

}



int main(void) {

	hw_init();

	while (1) {
		Estados();
		Ativa_Saidas();
	}

}

void Init_Time() {
	TCCR1B = 0;
	TIFR1 = (7 << TOV1) | (1 << ICF1);
	TCCR1A = 0;
	TCNT1 = T1BOTTOM;
	TIMSK1 = (1 << TOIE1);
	TCCR1B = 4;

}

void Start_Time(uint16_t Time_ms, uint8_t escolha) {
	/*Recebemos o tempo em ms, como o ciclo é de 10ms, então temos de fazer a divisão por 10.
	 * Ex: 5000ms = 500 ciclos de 10ms */
	Time_ms= Time_ms/10;

	switch (escolha) {
	case VERDE:
		T_Verde = Time_ms;
		T_Amarelo = -1;
		T_Vermelho = -1;
		T_Emergencia = -1;
		break;
	case AMARELO:
		T_Amarelo = Time_ms;
		T_Verde = -1;
		T_Vermelho = -1;
		T_Emergencia = -1;
		break;
	case VERMELHO:
		T_Vermelho = Time_ms;
		T_Verde = -1;
		T_Amarelo = -1;
		T_Emergencia = -1;
		break;
	case 4:
		T_Emergencia = Time_ms;
		T_Vermelho = -1;
		T_Verde = -1;
		T_Amarelo = -1;
		break;
	}
}

void Testa_Leds(void) {
	PORTB = (1 << GREEN_N);
	_delay_ms(200);
	PORTB = (1 << YELLOW_N);
	_delay_ms(200);
	PORTB = (1 << RED_N);
	_delay_ms(200);
	PORTB = (1 << GREEN_E);
	_delay_ms(200);
	PORTB = (1 << YELLOW_E);
	_delay_ms(200);
	PORTB = (1 << RED_E);
	_delay_ms(200);

	PORTB = (1 << GREEN_N) | (1 << YELLOW_N) | (1 << RED_N) | (1 << GREEN_E)
			| (1 << YELLOW_E) | (1 << RED_E);
	_delay_ms(1000);
	PORTB = 0;
	_delay_ms(500); /*espera um pouco antes de começar o programa*/
}

void hw_init(void) {
	DDRB = (1 << RED_N) | (1 << YELLOW_N) | (1 << GREEN_N) | (1 << YELLOW_E)
			| (1 << GREEN_E) | (1 << RED_E);

	DDRD = DDRD & ~(1 << EMERGENCIA);
	PORTD = PORTD | (1 << EMERGENCIA);

	EICRA = EICRA | (2 << ISC10);

	EIMSK = EIMSK | (1 << INT1);
	/*Estados iniciais*/
	estado = 1;
	flag = 0;

	/*Tempos*/
	Init_Time();
	sei();

	/*Testa LEDS*/
	Testa_Leds();

	Start_Time(TEMPO_VERDE, VERDE);

}

void Estados(void) {

	if (estado == 1 && !T_Verde) // tempo do verde N
			{
		estado = 2; /*Luz amarela N*/
		Start_Time(TEMPO_AMARELO, AMARELO);

	} else if (estado == 2 && !T_Amarelo) // tempo do amarelo N
			{
		estado = 3; /*Luz vermelha N*/
		Start_Time(TEMPO_VERMELHO, VERMELHO);

	} else if (estado == 3 && !T_Vermelho) // tempo do vermelho N
			{
		estado = 4; /*Luz verde E*/
		Start_Time(TEMPO_VERDE, VERDE);

	} else if (estado == 4 && !T_Verde) // tempo do verde E
			{
		estado = 5; /*Luz amarela E*/
		Start_Time(TEMPO_AMARELO, AMARELO);
	} else if (estado == 5 && !T_Amarelo) // tempo do amarelo E
			{
		estado = 6; /*Luz vermelha E*/
		Start_Time(TEMPO_VERMELHO, VERMELHO);
	} else if (estado == 6 && !T_Vermelho) // tempo do vermelho E
			{
		estado = 1; /* VERDE N*/
		Start_Time(TEMPO_VERDE, VERDE);
	}

	if (flag == 1) {
		/*Se tiver no Norte*/
		if (estado == 1) {
			estado = 2;
			Start_Time(TEMPO_AMARELO, AMARELO);
			flag = 2;
			/*Se tiver no Sul*/
		} else if(estado==2 || estado ==3)
			flag=2;
		else if (estado == 4) {
			estado = 5;
			Start_Time(TEMPO_AMARELO, AMARELO);
			flag = 3;
		} else if(estado==5 || estado==6)
			flag=3;
	}
}

void Erro(void) {
	while (1) {
		PORTB = (1 << YELLOW_N) | (1 << YELLOW_E);
		_delay_ms(1000);
		PORTB = 0;
		_delay_ms(1000);
	}
}

void Ativa_Saidas(void) {
	switch (estado) {

	case 1:
		/*ATIVA VERDE NORTE,
		 * ATIVA VERMELHO SUL
		 * Ativados: Verde norte e Vermelho SUl*/
		PORTB = ((1 << GREEN_N) | (1 << RED_E));
		break;

	case 2:
		/*ATIVA AMARELO NORTE,
		 * DESATIVA VERDE NORTE
		 * Ativados: Amarelo norte e Vermelho SUl*/
		PORTB = (PORTB & (~(1 << GREEN_N))) | ((1 << YELLOW_N));
		break;

	case 3:
		if (flag == 2) {
			PORTB = (1 << RED_N) | (1 << RED_E);
			_delay_ms(5000);
			flag = 0;
		} else {
			/*ATIVA VERMELHO NORTE,
			 * DESATIVA AMARELO NORTE
			 * Ativados: Vermelho Sul e Vermelho norte*/
			PORTB = (PORTB & (~(1 << YELLOW_N))) | ((1 << RED_N));
		}

		break;

	case 4:
		/*ATIVA VERDE SUL,
		 * DESATIVA VERMELHO SUL
		 * Ativados: Verde Sul e Vermelho norte*/
		PORTB = (PORTB & (~(1 << RED_E))) | ((1 << GREEN_E));
		break;

	case 5:
		/*ATIVA AMARELO SUL,
		 * DESATIVA VERDE SUL
		 * Ativados: Amarelo Sul e Vermelho norte*/
		PORTB = (PORTB & (~(1 << GREEN_E))) | ((1 << YELLOW_E));
		break;

	case 6:
		if (flag == 3) {
			PORTB = (1 << RED_N) | (1 << RED_E);
			_delay_ms(5000);
			flag = 0;
		} else {
			/*ATIVA VERMELHO SUL,
			 *DESATIVA AMARELO SUL
			 *Ativados: Vermelho sul e Vermelho norte*/
			PORTB = (PORTB & (~(1 << YELLOW_E))) | ((1 << RED_E));

		}
		break;

	default:
		/*PISCAR AMARELO A CADA SEGUNDO
		 * POSIÇÃO IRREGULAR ATINGIDA*/
		Erro();
		break;
	}

}

