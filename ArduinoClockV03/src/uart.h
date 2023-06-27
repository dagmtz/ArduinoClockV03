#ifndef UART_H_
#define UART_H_

#include <avr/io.h>

void UART_init();
void UART_write_char(unsigned char);
void UART_write_string(char*);
unsigned char UART_read();	

#endif /* UART_H_ */