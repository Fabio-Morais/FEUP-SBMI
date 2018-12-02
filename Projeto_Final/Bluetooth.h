#ifndef Blue
#define Blue
#include <avr/io.h>
#define F_CPU 16000000UL
#define BAUD 57600
#define UBBR_VAL ((F_CPU/(BAUD<<4))-1)

void init_usart(void){
	// Definir baudrate
	 UBRR0H = (uint8_t)(UBBR_VAL>>8);
	 UBRR0L = (uint8_t) UBBR_VAL;
	 // Definir formato da trama
	 UCSR0C = (3<<UCSZ00) // 8 data bits
	 | (0<<USBS0) // 1 stop bit
	 | (0<<UMSEL00) | (0<<UMSEL01);
	 // Ativar Rx, Tx
	 UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

#endif
