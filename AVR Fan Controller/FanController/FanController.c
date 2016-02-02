/*
 * new_rpm.c
 *
 * Created: 30.01.2016 20:12:16
 *  Author: Timm
 */ 


#define F_CPU 16000000
#define BAUD 38400

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <util/delay.h>

volatile char* tx_buf;
volatile char* rx_buf;
volatile char tx_len = 0;
volatile char rx_len = 0;
volatile char tx_flag = 1;
volatile char rx_flag = 0;

volatile char duty = 255;
volatile uint32_t int0_cycles = 0;

volatile char ext_int_flag = 0;
volatile char uart_flag = 0;

void pin_init(void) {
	DDRD &= ~(1 << DDD2); // INT0
	DDRD |= (1 << DDD5); // LED
	DDRD |= (1 << DDD6); // MOSFET
	
	PORTD |= (1 << PORTD2); // INT0 pullup
	
	//set PWM duty cycle
	OCR0B = duty; // LED
	OCR0A = duty; // MOSFET
	
	// set timer mode to fast PWM
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	
	//configure fast PWM in non-inverting mode
	TCCR0A |= (1 << COM0B1);
	TCCR0A |= (1 << COM0A1);
	
	// set prescaler to 1 and activate timer
	TCCR0B |= (1 << CS00);
	
	//INT0 setup
	EICRA |= (1 << ISC00); //trigger on falling edge
}

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

void enable_uart(){
	UCSR0B |= _BV(RXCIE0);
	UCSR0B |= _BV(RXEN0);
	UCSR0B |= _BV(TXEN0);   // Enable RX complete interrupt, RX and TX
	uart_flag = 1;
}

void disable_uart(){
	UCSR0B &= ~_BV(RXCIE0);
	UCSR0B &= ~_BV(RXEN0);
	UCSR0B &= ~_BV(TXEN0);   // Disable RX complete interrupt, RX and TX
	uart_flag = 0;
}

void enable_ext_int(){
	EIMSK |= (1 << INT0); //enable INT0
	ext_int_flag = 1;
}

void disable_ext_int(){
	EIMSK &= ~(1 << INT0); //disable INT0
	ext_int_flag = 0;
}

void send_data(char* data, char size){
	while(tx_flag != 1 || uart_flag != 1){}
	tx_len = size;
	tx_buf = data;
	tx_flag = 0;
	UCSR0B |= _BV(UDRIE0); // enable USART Data Register Empty Interrupt
}

void send_string(char* data){
	char size = 0;
	char* temp = data;
	while (*temp != '\0' && size < 255)
	{
		temp++;
		size++;
	}
	send_data(data, size);
}

void measure_rpm(){
	while(tx_flag != 1){} // wait for pending uart transmission to finish
	disable_uart();
	enable_ext_int();
	
	char old_duty = OCR0A;
	int0_cycles = 0;
	
	int0_timer(&int0_cycles);
	
	OCR0A = old_duty;
	
	disable_ext_int();
	enable_uart();
}

extern void int0_timer(uint32_t* cycles);

int main(void)
{
	pin_init();
	uart_init();
	enable_uart();
	sei();
	while(1)
	{
		send_data(&int0_cycles, 4);
		measure_rpm();
		_delay_ms(1000);
		//OCR0A = 0x00;
		//OCR0B = 0x00;		
		//_delay_ms(100);
		//OCR0A = 0xFF;
		//OCR0B = 0xFF;
		//_delay_ms(900);
	}
}

ISR(USART_RX_vect)
{
	duty = UDR0;
	OCR0B = duty;
	OCR0A = duty;
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
		tx_flag = 1;
	}
}

ISR(INT0_vect)
{
	//OCR0B = 0xFF;
	//_delay_ms(1);
	//OCR0B = 0;
}