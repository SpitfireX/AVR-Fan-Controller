/*
 * UART.c
 *
 * Created: 02.02.2016 01:14:34
 *  Author: Timm
 */ 

#include "MCUConsts.h"
#include "UART.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#define RX_CHUNK_LENGTH 4

volatile char* tx_buf;
volatile char* rx_buf;
volatile char tx_len = 0;
volatile char rx_len = 0;

volatile char uart_tx_active = 0;
volatile char uart_rx_active = 0;
volatile char uart_enabled = 0;

volatile rx_callback_t rx_callback;

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
	if (!uart_enabled)
		return;
	
	while(uart_tx_active == 1){}
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

void uart_set_rx_callback(rx_callback_t callback){
	rx_callback = callback;
}

ISR(USART_RX_vect)
{
	//*(rx_buf++) = UDR0;
	//rx_len++;
	//
	//if (rx_len >= RX_CHUNK_LENGTH)
	//{
		//rx_callback(rx_buf, rx_len);
		//rx_len = 0;
	//}
	char temp = UDR0;
	OCR0A = temp;
	OCR0B = temp;
	OCR1AL = temp;
	OCR1BL = temp;
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