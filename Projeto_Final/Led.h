#ifndef LED
#define LED
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define ctrl PORTB // We are using port B
#define en PD2       // enable signal pin 2
#define rw PD1       // read/write signal pin 1
#define rs PD0       // register select signal pin 0

void lcd_command(unsigned char cmd);
void lcd_init(void);
void lcd_data(unsigned char data);
void lcdCommand(char);
void lcdData(char);
void lcd_print(unsigned char *str);


/*Separa 4 em 4 bits para enviar no modo 4bits*/
void lcdCommand(char cmd_value) {
  char cmd_value1;
  cmd_value1 = cmd_value & 0xF0;          // Mask lower nibble
  lcd_command(cmd_value1);                // Send to LCD
  cmd_value1 = ((cmd_value << 4) & 0xF0); // Shift 4-bit and mask
  lcd_command(cmd_value1);                // Send to LCD
}


void lcd_init(void) {
  lcdCommand(0x02); // To initialize LCD in 4-bit mode.
  _delay_ms(1);
  lcdCommand(0x28); // To initialize LCD in 2 lines, 5X7 dots and 4bit mode.
  _delay_ms(1);
  lcdCommand(0x01); // Clear LCD
  _delay_ms(1);
  lcdCommand(0x0E); // Turn on cursor ON
  _delay_ms(1);
  lcdCommand(0x80); // —8 go to first line and –0 is for 0th position
  _delay_ms(1);
  return;
}


void lcd_print(unsigned char *str) { // store address value of the string in pointer *str

	while (*str>0) {     // loop will go on till the NULL character in the string
    lcdData(*str++); // sending data on LCD byte by byte

  }
  return;
}
void lcdData(char data_value) {
  char data_value1;
  data_value1 = data_value & 0xF0;          // Mask lower nibble
  lcd_data(data_value1);                    // because PD4-PD7 pins are used.
  data_value1 = ((data_value << 4) & 0xF0); // Shift 4-bit and mask
  lcd_data(data_value1);                    // Send to LCD
}

void lcd_data(unsigned char data) {
  ctrl = data;
  ctrl |= (1 << rs);  // RS = 1 for data
  ctrl &= ~(1 << rw); // RW = 0 for write
  ctrl |= (1 << en);  // EN = 1 for High to Low pulse
  _delay_ms(1);
  ctrl &= ~(1 << en); // EN = 0 for High to Low Pulse
  _delay_ms(40);
  return;
}
void lcd_command(unsigned char cmd) {
  ctrl = cmd;
  ctrl &= ~(1 << rs); // RS = 0 for command
  ctrl &= ~(1 << rw); // RW = 0 for write
  ctrl |= (1 << en);  // EN = 1 for High to Low pulse
  _delay_ms(1);
  ctrl &= ~(1 << en); // EN = 0 for High to Low pulse
  _delay_ms(40);
  return;
}

#endif

