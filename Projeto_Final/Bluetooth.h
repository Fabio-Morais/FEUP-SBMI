#ifndef Blue
#define Blue
#include <avr/io.h>

#define BAUD 9600
#define UBBR_VAL (F_CPU/8/BAUD-1)

void init_usart(void){
	// Definir baudrate
	 UBRR0H = (unsigned char)(UBBR_VAL>>8);
	 UBRR0L = (unsigned char) UBBR_VAL;
	 // Definir formato da trama
	 UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) // 8 data bits
	 | (0<<USBS0); // 1 stop bit

	 UCSR0A= (1<<U2X0);
	 // Ativar Rx, Tx
	 UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

#endif
