
/*
 * PinTimer.S
 *
 * Created: 05.02.2016 05:10:25
 *  Author: Timm
 */ 

  #include <avr/io.h>

/* pin_timer(char reg, char mask, uint32_t* cycles);
 * reg: r24
 * mask: r22
 * cycles: r21, r20
 */
 .global pin_timer

/* measures the time between two falling edges on the input pin
 * needs two full signal cycles at worst
 */
pin_timer:
	push XL
	push XH
	push YL
	push YH
	mov XL, r24		; input register address
	clr XH
	mov YL, r20		; count variable pointer
	mov YH, r21
	rcall dummy_wait
	pop YH
	pop YL
	pop XH
	pop XL
	ret

/* waits for a falling edge on the input pin
 * while(LOW){}
 * while(HIGH){}
 */
dummy_wait:
dloop1:
	ld r18, X		; copy input pin into r18
	and r18, r22	; mask it
	cp r18, r22		; compare r18 with the mask
	brne dloop1		; loop until r18 and mask are equal
dloop2:
	ld r18, X
	and r18, r22
	cp r18, r22
	breq dloop2		; loop until r18 and mask are not equal
	ret