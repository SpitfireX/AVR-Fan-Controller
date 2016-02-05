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

volatile uint32_t int0_cycles = 0xAABBCCDD;

volatile char ch_num = 4;
volatile char ch_duty[4] = {255, 255, 255, 255};

void pin_init(void) {
	// input configuration
	// data direction as inputs
	DDRC &= ~(1 << DDC0); // channel 1
	DDRC &= ~(1 << DDC1); // channel 2
	DDRC &= ~(1 << DDC2); // channel 3
	
	// input pullups
	PORTC |= (1 << PORTC0); // channel 1
	PORTC |= (1 << PORTC1); // channel 2
	PORTC |= (1 << PORTC2); // channel 3
	
	// pwm configuration
	// data direction as outputs
	DDRD |= (1 << DDD6); // channel 1
	DDRD |= (1 << DDD5); // channel 2
	DDRB |= (1 << DDB1); // channel 3
	DDRB |= (1 << DDB2); // channel 4
	
	// set PWM duty cycles
	OCR0A = ch_duty[0]; // channel 1
	OCR0B = ch_duty[1]; // channel 2
	OCR1AL = ch_duty[2]; // channel 3
	OCR1BL = ch_duty[3]; // channel 4
	
	// Timer0 setup
	TCCR0A |= (1 << WGM01) | (1 << WGM00); // set timer mode to fast PWM
	TCCR0A |= (1 << COM0B1) | (1 << COM0A1); // configure fast PWM non-inverting mode
	TCCR0B |= (1 << CS00); // set prescaler to 1 and activate timer
	
	// Timer1 setup
	TCCR1A |= (1 << WGM10);
	TCCR1B |= (1 << WGM12); // set timer mode to 8-bit fast PWM
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1); // configure fast PWM non-inverting mode
	TCCR1B |= (1 << CS10); // set prescaler to 1 and activate timer
}

void measure_rpm(){
	while(uart_tx_active == 1){} // wait for pending uart transmission to finish
	uart_disable();
	
	int0_cycles = 0;
	while (!(PINC & (1 << PINC0))){}
	while (PINC & (1 << PINC0)){}
	while (!(PINC & (1 << PINC0)))
	{
		int0_cycles++;
	}
	while (PINC & (1 << PINC0))
	{
		int0_cycles++;
	}
	
	uart_enable();
}

extern void pin_timer(char reg, char mask, uint32_t* cycles);

int main(void)
{
	pin_init();
	uart_init();
	uart_enable();
	sei();
	while(1)
	{
		uart_send_data(&int0_cycles, 4);
		pin_timer(0x26, _BV(PINC0), &int0_cycles);
		//pin_timer(0xCC, 0xEE, 0xABAB);
		//measure_rpm();
		_delay_ms(1000);
	}
}