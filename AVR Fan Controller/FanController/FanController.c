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

#define CH_NUM 4

volatile char ch_duty[CH_NUM] = {255, 255, 255, 255};
volatile char* ch_ocr[CH_NUM] = {&OCR0A, &OCR0B, &OCR1AL, &OCR1BL};
volatile char ch_mask[CH_NUM] = {_BV(PINC0), _BV(PINC1), _BV(PINC2), _BV(PINC3)};
volatile uint32_t ch_cycles[CH_NUM];

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
	*ch_ocr[0] = ch_duty[0]; // channel 1
	*ch_ocr[1] = ch_duty[1]; // channel 2
	*ch_ocr[2] = ch_duty[2]; // channel 3
	*ch_ocr[3] = ch_duty[3]; // channel 4
	
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
	
	measure_channel_rpm(0); // channel 1
	measure_channel_rpm(1); // channel 2
	measure_channel_rpm(2); // channel 3
	measure_channel_rpm(3); // channel 4
	
	uart_enable();
}

void measure_channel_rpm(char channel){
	char old_duty = *ch_ocr[channel];
	*ch_ocr[channel] = 0xFF;
	pin_timer(&PINC, ch_mask[channel], &ch_cycles[channel]);
	*ch_ocr[channel] = old_duty;
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
		uart_send_data(ch_cycles, 4*CH_NUM);
		measure_rpm();
		_delay_ms(1000);
	}
}