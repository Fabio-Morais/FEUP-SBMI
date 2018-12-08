#ifndef Battery
#define Battery
#include <avr/io.h>
#include "Lcd.h"
#ifndef F_CPU
#define F_CPU 16000000ul
#endif

#define VREF 5


void init_adc(void) {
 // Definir Vref=AVcc e ADC0
 ADMUX = ADMUX | (1<<REFS0);
 // Desativar buffer digital em PC0
 DIDR0 = DIDR0 | (1<<PC0);
 // Pré-divisor em 128 e ativar ADC
 ADCSRA = ADCSRA | (7<<ADPS0)|(1<<ADEN);
}

unsigned int read_adc(unsigned char chan) {
 // escolher o canal...
 ADMUX = (ADMUX & 0xF0) | (chan & 0x0F);
 // iniciar a conversão
 // em modo manual (ADATE=0)
 ADCSRA |= (1<<ADSC);
 // esperar pelo fim da conversão
 while(ADCSRA & (1<<ADSC));
 return ADC;
}

double Read_Battery(unsigned int adc){
	double v;

	v= adc*VREF*1000/1024;

	return v;

}

/*Mostra percentagem bateria*/
void Print_Battery(double v){

	short i;

		for(i=0;i<10; i++)
		{
			lcd_gotoxy(i+7, 1);
			lcdData(0b11111111);
		}



}

/*ADC-245*10*/

#endif
