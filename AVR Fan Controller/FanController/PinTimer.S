
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
	; save all used registers
	push XL				; input register address pointer
	push XH
	push YL				; output variable pointer
	push YH
	push r2				; 4 bytes for the counter result
	push r3
	push r4
	push r5
	push r16			; cycles per iteration constant
	push r17			; cylces wasted between waits

	; prepare pointer registers
	mov XL, r24			; input register address pointer
	clr XH
	mov YL, r20			; output variable pointer
	mov YH, r21

	; zero counter variable
	clr r2
	clr r3
	clr r4
	clr r5

	; prepare cycles constant
	ldi r16, 16			; -> cycle counts.txt
	ldi r17, 7			; see code below

	; measure cycles
	rcall counter_wait	; wait for first falling edge

	; check for error/early abort (r5 > 0)
	cp r5, r1			; r5 == r1
	brne pt_exit		; skip 2nd measurement

	; zero counter variable
	clr r2
	clr r3
	clr r4
	clr r5
	mov r2, r17			; compensate for zeroing

	rcall counter_wait	; wait for second falling edge

pt_exit:
	; write output variable
	st Y+, r2
	st Y+, r3
	st Y+, r4
	st Y, r5

	; restore all registers
	pop r17
	pop r16
	pop r5
	pop r4
	pop r3
	pop r2
	pop YH
	pop YL
	pop XH
	pop XL
	ret

/* waits for a falling edge on the input pin while counting the
 * elapsed cycles. Times out after a certain number of cycles.
 * while(LOW){cycles++; if(cylces > MAX) error;}
 * while(HIGH){cycles++; if(cylces > MAX) error;}
 */
counter_wait:
cloop1:
	rcall add_cycles
	brcs cw_error		; timeout error if cycle count > 0x00FFFFFF
	ld r18, X			; copy input pin into r18
	and r18, r22		; mask it
	cp r18, r22			; compare r18 with the mask
	brne cloop1			; loop until r18 and mask are equal
cloop2:
	rcall add_cycles
	brcs cw_error
	ld r18, X
	and r18, r22
	cp r18, r22
	breq cloop2			; loop until r18 and mask are not equal
cw_exit:
	ret
cw_error:
	inc r5
	ret

add_cycles:
	add r2, r16
	adc r3, r1
	adc r4, r1
	ret