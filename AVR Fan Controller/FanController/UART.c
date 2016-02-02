/*
 * UART.c
 *
 * Created: 02.02.2016 01:14:34
 *  Author: Timm
 */ 

#define F_CPU 16000000
#define BAUD 38400

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

volatile char* tx_buf;
volatile char* rx_buf;
volatile char tx_len = 0;
volatile char rx_len = 0;

volatile char uart_tx_active = 0;
volatile char uart_rx_active = 0;
volatile char uart_enabled = 0;

void uart_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8-bit data
}

void uart_enable(void){
	UCSR0B |= _BV(RXCIE0);
	UCSR0B |= _BV(RXEN0);
	UCSR0B |= _BV(TXEN0);   // Enable RX complete interrupt, RX and TX
	uart_enabled = 1;
}

void uart_disable(void){
	UCSR0B &= ~_BV(RXCIE0);
	UCSR0B &= ~_BV(RXEN0);
	UCSR0B &= ~_BV(TXEN0);   // Disable RX complete interrupt, RX and TX
	uart_enabled = 0;
}

void uart_send_data(char* data, char size){
	while(uart_tx_active == 1 || uart_enabled != 1){}
	tx_len = size;
	tx_buf = data;
	uart_tx_active = 1;
	UCSR0B |= _BV(UDRIE0); // enable USART Data Register Empty Interrupt
}

void uart_send_string(char* data){
	char size = 0;
	char* temp = data;
	while (*temp != '\0' && size < 255)
	{
		temp++;
		size++;
	}
	uart_send_data(data, size);
}

ISR(USART_RX_vect)
{
}

ISR(USART_UDRE_vect)
{
	if (tx_len > 0)
	{
		tx_len--;
		char data = *(tx_buf++);
		UDR0 = data;
	}
	else
	{
		UCSR0B &= ~_BV(UDRIE0); // disable USART Data Register Empty Interrupt
		uart_tx_active = 0;
	}
}