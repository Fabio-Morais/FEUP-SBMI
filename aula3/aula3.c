/*******************************************
 *  aula.c
 *  Created on: 06/10/2018 (eclipse, avr-gcc)
 *      Author: up201504257@fe.up.pt
 *
 *      Estado 1-> LUZ VERDE NORTE, LUZ VERMELHA SUL
 *      Estado 2-> LUZ AMARELA NORTE, LUZ VERMELHA SUL
 *      Estado 3-> LUZ VERMELHA NORTE, LUZ VERMELHA SUL
 *      Estado 4-> LUZ VERDE SUL, LUZ VERMELHA NORTE
 *      Estado 5-> LUZ AMARELA SUL, LUZ VERMELHA NORTE
 *      Estado 6-> LUZ VERMELHA SUL, LUZ VERMELHA NORTE
 *
 *
 *	Solução: Decrementar duas variaveis a cada 10ms, uma variavel para o LED verde e outra variavel para o
 *			 LED amarelo e vermelho, pois para estes 2 o tempo é o mesmo.
 *******************************************
 *	A melhor combinação de COUNT que achamos foi:
 *	CP=1, TP=256, COUNT=625
 *******************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*LEDS SUL*/
#define RED_E PB5
#define YELLOW_E PB4
#define GREEN_E PB3

/*LEDS NORTE*/
#define RED_N PB2
#define YELLOW_N PB1
#define GREEN_N PB0

/**Botao emergencia*/
#define EMERGENCIA PD3

/* 10ms = 625 clock cycles
 * CNT= (16M * 10ms / 256)
 * CNT= 625 */
#define T1BOTTOM 65536-625

/*Luz para cada numero, para nos organizarmos*/
#define VERDE 1
#define AMARELO 2
#define VERMELHO 3

/*Tempos para o timer*/
#define TEMPO_VERDE 100
#define TEMPO_AMARELO_VERMELHO 100

/*Tempos suplentes*/
#define SHORT 200
#define LONG 1000

uint8_t estado;
volatile uint8_t flag;
volatile uint16_t T_Verde, T_Amarelo_Vermelho, T_Emergencia;

/*Faz inicialiazação do Timer 1*/
void Init_Time(void);

/* Iniciação das variaveis e timers */
void hw_init(void);

/*Dá START no timer escolhido */
void Start_Time(uint8_t Time, uint8_t escolha);

/* Transições entre estados*/
void Estados(void);

/* Ativa as saidas (LEDS) de acordo com os estados*/
void Ativa_Saidas(void);

/*Testa funcionamento de todos os LEDS */
void Testa_Leds(void);

/* Pisca o LED amarelo, posição irregular*/
void Erro(void);

/*Interrupção do botão*/
ISR(INT1_vect) {
	flag = 1;
}

/*Timer 1 ISR executa a cada 10ms*/
ISR(TIMER1_OVF_vect) {

	/*Reload TC1*/
	TCNT1 = T1BOTTOM;

	/*É possivel decrementar varios timers ao mesmo tempo assim*/
	if (T_Verde)
		T_Verde--;

	if (T_Amarelo_Vermelho)
		T_Amarelo_Vermelho--;

}

int main(void) {

	hw_init();

	while (1) {
		Estados();
		Ativa_Saidas();
	}

}

void Init_Time(void) {
	TCCR1B = 0; 					   //Stop Tcc1
	TIFR1 = (7 << TOV1) | (1 << ICF1); //Clear all pending interrupts
	TCCR1A = 0; 					   //NORMAL mode
	TCNT1 = T1BOTTOM; 				   //Load BOTTOM value ( 65536-625)
	TIMSK1 = (1 << TOIE1);			   //Enable 0vf intrpt
	TCCR1B = 4;						   // Start TC1 (TP=256)

}

void hw_init(void) {

	/*Inicializa as portas para Output*/
	DDRB = (1 << RED_N) | (1 << YELLOW_N) | (1 << GREEN_N) | (1 << YELLOW_E)
			| (1 << GREEN_E) | (1 << RED_E);

	/*Ativar o Pull Up*/
	DDRD = DDRD & ~(1 << EMERGENCIA);
	PORTD = PORTD | (1 << EMERGENCIA);

	/*INT1 Falling edfe*/
	EICRA = EICRA | (2 << ISC10);

	/*Ativar INT1*/
	EIMSK = EIMSK | (1 << INT1);

	/*Estados iniciais*/
	estado = 1;
	flag = 0;

	/*Tempos*/
	Init_Time();
	sei();

	/*Testa LEDS*/
	Testa_Leds();

	/*Começa a contar o timer do verde*/
	Start_Time(TEMPO_VERDE, VERDE);

}

void Start_Time(uint8_t Time, uint8_t escolha) {

	switch (escolha) {
	case VERDE:
		T_Verde = Time;
		break;
	case AMARELO:
		T_Amarelo_Vermelho = Time;
		break;
	case VERMELHO:
		T_Amarelo_Vermelho = Time;
		break;
	case 4:
		T_Emergencia = Time;
		break;
	default:
		Erro();
		break;
	}
}

void Estados(void) {

	if (estado == 1 && !T_Verde) // tempo do verde N
			{
		estado = 2; /*Luz amarela N*/
		Start_Time(TEMPO_AMARELO_VERMELHO, AMARELO);

	} else if (estado == 2 && !T_Amarelo_Vermelho) // tempo do amarelo N
			{
		estado = 3; /*Luz vermelha N*/
		Start_Time(TEMPO_AMARELO_VERMELHO, VERMELHO);

	} else if (estado == 3 && !T_Amarelo_Vermelho) // tempo do vermelho N
			{
		estado = 4; /*Luz verde E*/
		Start_Time(TEMPO_VERDE, VERDE);

	} else if (estado == 4 && !T_Verde) // tempo do verde E
			{
		estado = 5; /*Luz amarela E*/
		Start_Time(TEMPO_AMARELO_VERMELHO, AMARELO);
	} else if (estado == 5 && !T_Amarelo_Vermelho) // tempo do amarelo E
			{
		estado = 6; /*Luz vermelha E*/
		Start_Time(TEMPO_AMARELO_VERMELHO, VERMELHO);
	} else if (estado == 6 && !T_Amarelo_Vermelho) // tempo do vermelho E
			{
		estado = 1; /* VERDE N*/
		Start_Time(TEMPO_VERDE, VERDE);
	}

	/* flag==1 Ocorreu interrupçao do botão
	 * flag==2 ocorreu no lado NORTE
	 * flag==3 ocorreu no lado SUL
	 * */
	if (flag == 1) {

		/*Se tiver no Verde Norte*/
		if (estado == 1) {
			estado = 2;
			Start_Time(TEMPO_AMARELO_VERMELHO, AMARELO);
			flag = 2;
		}

		/*Se tiver no Amarelo ou Vermelho Norte*/
		else if (estado == 2 || estado == 3)
			flag = 2;

		/*Se tiver no Verde Sul*/
		else if (estado == 4) {
			estado = 5;
			Start_Time(TEMPO_AMARELO_VERMELHO, AMARELO);
			flag = 3;
		}

		/*Se tiver no amarelo ou vermelho SUl*/
		else if (estado == 5 || estado == 6)
			flag = 3;
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

void Testa_Leds(void) {
	PORTB = (1 << GREEN_N);
	_delay_ms(SHORT);
	PORTB = (1 << YELLOW_N);
	_delay_ms(SHORT);
	PORTB = (1 << RED_N);
	_delay_ms(SHORT);
	PORTB = (1 << GREEN_E);
	_delay_ms(SHORT);
	PORTB = (1 << YELLOW_E);
	_delay_ms(SHORT);
	PORTB = (1 << RED_E);
	_delay_ms(SHORT);

	/*Ativa tudo*/
	PORTB = (1 << GREEN_N) | (1 << YELLOW_N) | (1 << RED_N) | (1 << GREEN_E)
			| (1 << YELLOW_E) | (1 << RED_E);

	_delay_ms(LONG);
	PORTB = 0;
	_delay_ms(LONG - SHORT); /*espera um pouco antes de começar o programa*/
}

void Erro(void) {
	while (1) {
		PORTB = (1 << YELLOW_N) | (1 << YELLOW_E);
		_delay_ms(LONG);
		PORTB = 0;
		_delay_ms(LONG);
	}
}

