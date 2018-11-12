/*******************************************
 *  aula3.c
 *  Created on: 06/10/2018 (eclipse, avr-gcc)
 *      Author: up201504257@fe.up.pt
 *      		up201708979@fe.up.pt
 *
 *	Objetivo: Implementar dois semaforos, um semaforo norte e um semaforo sul
 *			  em que está 50s no verde norte, 5s no amarelo norte e depois 5s no vermelho, em seguida
 *			  ativar o lado sul com os mesmos tempos.
 *
 *	Solução: Decrementar duas variaveis a cada 10ms, uma variavel para o LED verde e outra variavel para o
 *			 LED amarelo e vermelho, pois para estes 2 o tempo é o mesmo.
 *			  *
 *      Estado 1-> LUZ VERDE NORTE, LUZ VERMELHA SUL
 *      Estado 2-> LUZ AMARELA NORTE, LUZ VERMELHA SUL
 *      Estado 3-> LUZ VERMELHA NORTE, LUZ VERMELHA SUL
 *      Estado 4-> LUZ VERDE SUL, LUZ VERMELHA NORTE
 *      Estado 5-> LUZ AMARELA SUL, LUZ VERMELHA NORTE
 *      Estado 6-> LUZ VERMELHA SUL, LUZ VERMELHA NORTE
 *
 *      Estado_emergencia 0-> estado normal de funcionamento
 *      Estado_emergencia 1-> estado em modo de emergencia, fica 10s vermelho dos 2 lados
 *******************************************
 *	A melhor combinação de COUNT que achamos foi:
 *	CP=1, TP=256, COUNT=625
 *******************************************/
#define DEBUG

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serial_printf.h"

/*Semaforo SUL*/
#define RED_E PB5
#define YELLOW_E PB4
#define GREEN_E PB3

/*Semaforo NORTE*/
#define RED_N PB2
#define YELLOW_N PB1
#define GREEN_N PB0

/**Botao emergencia*/
#define BTN_EMERGENCIA PD3

/* 10ms = 625 clock cycles
 * CNT= (16M * 10ms / 256)
 * CNT= 625 */
#define T1BOTTOM 65536-625

/*Luz para cada numero, para nos organizarmos*/
#define VERDE 1
#define AMARELO 2
#define VERMELHO 3
#define EMERGENCIA 4

/*******************************************/
#ifdef DEBUG
/*Tempos para o timer*/
#define TEMPO_VERDE 100
#define TEMPO_AMARELO_VERMELHO 100
#define TEMPO_EMERGENCIA 400
#endif

#ifdef NORMAL
/*Tempos para o timer*/
#define TEMPO_VERDE 5000
#define TEMPO_AMARELO_VERMELHO 500
#define TEMPO_EMERGENCIA 1000
#endif
/*******************************************/

/*Tempos suplentes*/
#define SHORT 200
#define LONG 1000

uint8_t estado, estado_emergencia;

#ifdef DEBUG
uint8_t Pre_estado, Pre_estado_emergencia;
#endif;

volatile uint8_t flag;
volatile uint16_t T_Verde, T_Amarelo_Vermelho, T_Emergencia;

/*Faz inicialiazação do Timer 1*/
void Init_Time(void);

/* Iniciação das variaveis e timers */
void hw_init(void);

/*Dá START no timer escolhido */
void Start_Time(uint16_t Time, uint8_t escolha);

/* Transições entre estados*/
void Estados(void);

/* Ativa as saidas (Luzes) de acordo com os estados*/
void Ativa_Saidas(void);

/*Testa funcionamento de todas as luzes dos semaforos */
void Testa_Leds(void);

/* Pisca a luz amarela, posição irregular*/
void Erro(void);

/*Interrupção do botão*/
ISR(INT1_vect) {
	flag = 1;
}

/*Timer 1 ISR executa a cada 10ms*/
ISR(TIMER1_OVF_vect) {

	/*Reload TC1*/
	TCNT1 = T1BOTTOM;

	/*É possivel decrementar varios timers ao mesmo tempo */
	if (T_Verde)
		T_Verde--;

	if (T_Amarelo_Vermelho)
		T_Amarelo_Vermelho--;

	if (T_Emergencia)
		T_Emergencia--;
}

int main(void) {

	hw_init();

	while (1) {
		Estados();
		Ativa_Saidas();


		#ifdef DEBUG
		if(estado!= Pre_estado || estado_emergencia != Pre_estado_emergencia)
			printf("Normal:%d\n Emergencia: %d\n", estado, estado_emergencia);
		Pre_estado=estado;
		Pre_estado_emergencia=estado_emergencia;
		#endif;
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
	DDRD = DDRD & ~(1 << BTN_EMERGENCIA);
	PORTD = PORTD | (1 << BTN_EMERGENCIA);

	/*INT1 Falling Edge*/
	EICRA = EICRA | (1 << ISC11) ;

	/*Ativar INT1*/
	EIMSK = EIMSK | (1 << INT1);

	/*Estados iniciais*/
	estado = 1;
	estado_emergencia = 0;
	flag = 0;

	/*Tempos*/
	Init_Time();
	sei();
	T_Verde = T_Amarelo_Vermelho = T_Emergencia = 0;

	#ifdef DEBUG
	/*Biblioteca printf*/
	printf_init();
	printf("Hello world");
	#endif;

	/*Testa luzes dos semaforos*/
	Testa_Leds();

	/*Começa a contar o timer do verde*/
	Start_Time(TEMPO_VERDE, VERDE);
}

void Start_Time(uint16_t Time, uint8_t escolha) {

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
		if (estado_emergencia) //Caso tenha carregado no botão
			Start_Time(TEMPO_EMERGENCIA, EMERGENCIA); //começa a contar o tempo de emergencia(10s)
		else
			Start_Time(TEMPO_AMARELO_VERMELHO, VERMELHO);//começa a contar o tempo normal(5s)

	} else if (estado == 3 && ((!T_Amarelo_Vermelho && !estado_emergencia) || (!T_Emergencia && estado_emergencia ))) // tempo do vermelho N
			{
		estado = 4; /*Luz verde E*/
		estado_emergencia = 0;
		Start_Time(TEMPO_VERDE, VERDE);

	} else if (estado == 4 && !T_Verde) // tempo do verde E
			{
		estado = 5; /*Luz amarela E*/
		Start_Time(TEMPO_AMARELO_VERMELHO, AMARELO);
	} else if (estado == 5 && !T_Amarelo_Vermelho) // tempo do amarelo E
			{
		estado = 6; /*Luz vermelha E*/
		if (estado_emergencia)//Caso tenha carregado no botão
			Start_Time(TEMPO_EMERGENCIA, EMERGENCIA);//começa a contar o tempo de emergencia(10s)
		else
			Start_Time(TEMPO_AMARELO_VERMELHO, VERMELHO);//começa a contar o tempo normal(5s)

	} else if (estado == 6 && ((!T_Amarelo_Vermelho && !estado_emergencia) || (!T_Emergencia && estado_emergencia ))) // tempo do vermelho E
			{
		estado = 1; /* VERDE N*/
		estado_emergencia = 0;
		Start_Time(TEMPO_VERDE, VERDE);
	}

	/* flag==1 Ocorreu interrupçao do botão
	 * flag==0 Não ocorreu interrupção do botão*/
	if (flag) {

		/*Se estiver no Verde Norte*/
		if (estado == 1) {
			estado = 2;
			Start_Time(TEMPO_AMARELO_VERMELHO, AMARELO); /*ativa o timer de amarelo*/
			estado_emergencia = 1;

			/*Se estiver no amarelo N ou amarelo S*/
		} else if (estado == 2 || estado == 5)
			estado_emergencia = 1;

		/*Se estiver no Vermelho Norte*/
		else if (estado == 3) {
			Start_Time(TEMPO_EMERGENCIA, EMERGENCIA); /*ativa o timer de emergencia*/
			estado_emergencia = 1;

			/*Se estiver no verde Sul*/
		} else if (estado == 4) {
			estado = 5;
			Start_Time(TEMPO_AMARELO_VERMELHO, AMARELO); /*ativa o timer amarelo*/
			estado_emergencia = 1;
		}

		/*Se estiver no vermelho SUl*/
		else if (estado == 6) {
			Start_Time(TEMPO_EMERGENCIA, EMERGENCIA); /*ativa o timer de emergencia*/
			estado_emergencia = 1;
		}
		flag = 0;
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
		 * **Ativados: Amarelo norte e Vermelho SUl** */
		PORTB = (PORTB & (~(1 << GREEN_N))) | ((1 << YELLOW_N));
		break;

	case 3:
		/*ATIVA VERMELHO NORTE,
		 * DESATIVA AMARELO NORTE
		 * **Ativados: Vermelho Sul e Vermelho norte** */
		PORTB = (PORTB & (~(1 << YELLOW_N))) | ((1 << RED_N));

		break;

	case 4:
		/*ATIVA VERDE SUL,
		 * DESATIVA VERMELHO SUL
		 * **Ativados: Verde Sul e Vermelho norte** */
		PORTB = (PORTB & (~(1 << RED_E))) | ((1 << GREEN_E));
		break;

	case 5:
		/*ATIVA AMARELO SUL,
		 * DESATIVA VERDE SUL
		 * **Ativados: Amarelo Sul e Vermelho norte** */
		PORTB = (PORTB & (~(1 << GREEN_E))) | ((1 << YELLOW_E));
		break;

	case 6:
		/*ATIVA VERMELHO SUL,
		 *DESATIVA AMARELO SUL
		 * **Ativados: Vermelho sul e Vermelho norte** */
		PORTB = (PORTB & (~(1 << YELLOW_E))) | ((1 << RED_E));
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

