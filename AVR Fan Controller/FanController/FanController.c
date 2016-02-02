/*
 * new_rpm.c
 *
 * Created: 30.01.2016 20:12:16
 *  Author: Timm
 */ 

#include "MCUConsts.h"
#include "UART.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile char duty = 255;
volatile uint32_t int0_cycles = 0;

volatile char ext_int_flag = 0;

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

void enable_ext_int(){
	EIMSK |= (1 << INT0); //enable INT0
	ext_int_flag = 1;
}

void disable_ext_int(){
	EIMSK &= ~(1 << INT0); //disable INT0
	ext_int_flag = 0;
}

void measure_rpm(){
	while(uart_tx_active == 1){} // wait for pending uart transmission to finish
	uart_disable();
	enable_ext_int();
	
	char old_duty = OCR0A;
	int0_cycles = 0;
	
	int0_timer(&int0_cycles);
	
	OCR0A = old_duty;
	
	disable_ext_int();
	uart_enable();
}

extern void int0_timer(uint32_t* cycles);

int main(void)
{
	pin_init();
	uart_init();
	uart_enable();
	sei();
	while(1)
	{
		uart_send_data(&int0_cycles, 4);
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

ISR(INT0_vect)
{
	//OCR0B = 0xFF;
	//_delay_ms(1);
	//OCR0B = 0;
}