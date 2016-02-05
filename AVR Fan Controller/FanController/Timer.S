
/*
 * timer.S
 *
 * Created: 30.01.2016 21:30:19
 *  Author: Timm
 */ 

 #include <avr/io.h>

 .global int0_timer

 add_counter:
	ldi r18, 19		; ~ 19 cycles per iteration
	subi XL, -3

	ld r19, X 
	add r19, r18
	st X, r19

	ld r19, -X 
	adc r19, r1
	st X, r19

	ld r19, -X
	adc r19, r1
	st X, r19

	ld r19, -X
	adc r19, r1
	st X, r19

	ret

measure_wait:		; measure the time between the inital trigger and the next
mw_loop:
	rcall add_counter
	sbis 0x1C, 0
	rjmp mw_loop
	ret

initial_wait:		; wait for an initial external interrupt trigger
iw_loop:
	sbis 0x1C, 0
	rjmp iw_loop
	rcall measure_wait
	ret

int0_timer:
	push XL
	push XH
	mov XL, r24
	mov XH, r25
	rcall initial_wait
	pop XH
	pop XL
	ret