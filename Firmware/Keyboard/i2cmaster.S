;*************************************************************************
; Title	:    I2C (Single) Master Implementation
; Author:    Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
;            based on Atmel Appl. Note AVR300
; File:      $Id: i2cmaster.S,v 1.12 2008/03/02 08:51:27 peter Exp $
; Software:  AVR-GCC 3.3 or higher
; Target:    any AVR device
;
; DESCRIPTION
; 	Basic routines for communicating with I2C slave devices. This
;	"single" master implementation is limited to one bus master on the
;	I2C bus. 
;  
;       Based on the Atmel Application Note AVR300, corrected and adapted 
;       to GNU assembler and AVR-GCC C call interface
;       Replaced the incorrect quarter period delays found in AVR300 with
;       half period delays.

; Tweaked by monty for true 400KHz operation at 8MHz system clock on an
;  atmega16u2. Duty cycle adjusted for correct 400KHz spec adherence.
;
; USAGE
;	These routines can be called from C, refere to file i2cmaster.h.
;       See example test_i2cmaster.c 
; 	Adapt the SCL and SDA port and pin definitions and eventually 
;	the delay routine to your target !
; 	Use 4.7k pull-up resistor on the SDA and SCL pin.
;
; NOTES
;	The I2C routines can be called either from non-interrupt or
;	interrupt routines, not both.
;
;*************************************************************************

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 303
#error "This library requires AVR-GCC 3.3 or later, update to newer AVR-GCC compiler !"
#endif


#include <avr/io.h>



;***** Adapt these SCA and SCL port and pin definition to your target !!
;
; V1 had different I2C pinouts
#ifdef V1_BUILD
    #define SDA      1 ; SDA Port D, Pin 4
    #define SCL      0 ; SCL Port D, Pin 5
    #define SDA_PORT PORTD
    #define SCL_PORT PORTD
#else
    #define SDA      4 ; SDA Port D, Pin 4
    #define SCL      3 ; SCL Port D, Pin 3
    #define SDA_PORT PORTD
    #define SCL_PORT PORTD
#endif

;******

;-- map the IO register back into the IO address space
#define SDA_DDR		(_SFR_IO_ADDR(SDA_PORT) - 1)
#define SCL_DDR		(_SFR_IO_ADDR(SCL_PORT) - 1)
#define SDA_OUT		_SFR_IO_ADDR(SDA_PORT)
#define SCL_OUT		_SFR_IO_ADDR(SCL_PORT)
#define SDA_IN		(_SFR_IO_ADDR(SDA_PORT) - 2)
#define SCL_IN		(_SFR_IO_ADDR(SCL_PORT) - 2)


#ifndef __tmp_reg__
#define __tmp_reg__ 0
#endif


	.section .text

;*************************************************************************
; delay half period
; For I2C in normal mode (100kHz), use T/2 > 5us
; For I2C in fast mode (400kHz),   use T/2 > 1.3us
;*************************************************************************
	.stabs	"",100,0,0,i2c_delay_T2
	.stabs	"i2cmaster.S",100,0,0,i2c_delay_T2
	.func i2c_delay_T2	; delay 1.25 microsec with 8 Mhz crystal
i2c_delay_T2:        ; 4 cycles
	ret          ; 5   "
	.endfunc     ; total 9 cyles = 1.25 microsec with 8 Mhz crystal, IO op happens on the 10th
    
;*************************************************************************
; delay 40 microseconds, to give the slave time to do stuff
;*************************************************************************
	.func i2c_delay_40us
i2c_delay_40us:        ; 4 cycles
    ldi r16, 103      ; 1
delay:
    dec r16          ; 1
    brne delay       ; 2
    ; 1
	ret          ; 5   "
	.endfunc     ; total 320 cyles = 40 microsec with 8 Mhz crystal


;*************************************************************************
; Initialization of the I2C bus interface. Need to be called only once
;
; extern void i2c_init(void)
;*************************************************************************
	.global i2c_init
	.func i2c_init
i2c_init:
	cbi SDA_DDR,SDA		;release SDA
	cbi SCL_DDR,SCL		;release SCL
	cbi SDA_OUT,SDA
	cbi SCL_OUT,SCL
	ret
	.endfunc


;*************************************************************************	
; Issues a start condition and sends address and transfer direction.
; return 0 = device accessible, 1= failed to access device
;
; extern unsigned char i2c_start(unsigned char addr);
;	addr = r24, return = r25(=0):r24
;*************************************************************************

	.global i2c_start
	.func   i2c_start
i2c_start:
	sbi 	SDA_DDR,SDA	;force SDA low
	rcall 	i2c_delay_T2	;delay T/2
	
	rcall 	i2c_write	;write address
	ret
	.endfunc		


;*************************************************************************
; Issues a repeated start condition and sends address and transfer direction.
; return 0 = device accessible, 1= failed to access device
;
; extern unsigned char i2c_rep_start(unsigned char addr);
;	addr = r24,  return = r25(=0):r24
;*************************************************************************

	.global i2c_rep_start
	.func	i2c_rep_start
i2c_rep_start:
	sbi	SCL_DDR,SCL	;force SCL low
	rcall 	i2c_delay_T2	;delay  T/2
	cbi	SDA_DDR,SDA	;release SDA
	rcall	i2c_delay_T2	;delay T/2
	cbi	SCL_DDR,SCL	;release SCL
	rcall 	i2c_delay_T2	;delay  T/2
	sbi 	SDA_DDR,SDA	;force SDA low
	rcall 	i2c_delay_T2	;delay	T/2
	
	rcall	i2c_write	;write address
	ret
	.endfunc


;*************************************************************************	
; Issues a start condition and sends address and transfer direction.
; If device is busy, use ack polling to wait until device is ready
;
; extern void i2c_start_wait(unsigned char addr);
;	addr = r24
;*************************************************************************

	.global i2c_start_wait
	.func   i2c_start_wait
i2c_start_wait:
	mov	__tmp_reg__,r24
i2c_start_wait1:
	sbi 	SDA_DDR,SDA	;force SDA low
	rcall 	i2c_delay_T2	;delay T/2
	mov	r24,__tmp_reg__
	rcall 	i2c_write	;write address
	tst	r24		;if device not busy -> done
	breq	i2c_start_wait_done
	rcall	i2c_stop	;terminate write operation
	rjmp	i2c_start_wait1	;device busy, poll ack again
i2c_start_wait_done:
	ret
	.endfunc	


;*************************************************************************
; Terminates the data transfer and releases the I2C bus
;
; extern void i2c_stop(void)
;*************************************************************************

	.global	i2c_stop
	.func	i2c_stop
i2c_stop:
	sbi	SCL_DDR,SCL	;force SCL low
	sbi	SDA_DDR,SDA	;force SDA low
	rcall	i2c_delay_T2	;delay T/2
	cbi	SCL_DDR,SCL	;release SCL
	rcall	i2c_delay_T2	;delay T/2
i2c_stop_wait:
    ;wait SCL high (in case wait states are inserted)
	sbis	SCL_IN,SCL                                ; 2
	rjmp	i2c_stop_wait
	cbi	SDA_DDR,SDA	;release SDA
	rcall	i2c_delay_T2	;delay T/2
    rcall i2c_delay_40us ; delay 40us to let slave catch its breath
	ret
	.endfunc


;*************************************************************************
; Send one byte to I2C device
; return 0 = write successful, 1 = write failed
;
; extern unsigned char i2c_write( unsigned char data );
;	data = r24,  return = r25(=0):r24
;*************************************************************************
	.global i2c_write
	.func	i2c_write
i2c_write:
	sec			;set carry flag                       ;
	rol 	r24		;shift in carry and out bit one   ;
	rjmp	i2c_write_first                           ;
i2c_write_bit:
	lsl	r24		;if transmit register empty           ; 1
	breq	i2c_get_ack                               ; 1
    nop                                               ; 1
i2c_write_first:
    ; 7 since the last cbi
	sbi	SCL_DDR,SCL	;force SCL low                    ; 2
	brcc	i2c_write_low                             ; 1
    nop                                               ; 1
	cbi	SDA_DDR,SDA	;release SDA                      ; 2
	rjmp	i2c_write_high                            ; 2
i2c_write_low:                                        ; 1
	sbi	SDA_DDR,SDA	;force SDA low                    ; 2
	rjmp	i2c_write_high                            ; 2
i2c_write_high:
    nop
    nop
    nop
    nop
    nop
    ; 13 since the last
	cbi	SCL_DDR,SCL	;release SCL                      ; 2
i2c_write_wait:
    ;wait SCL high (in case wait states are inserted)
	sbis	SCL_IN,SCL                                ; 2
	rjmp	i2c_write_wait

	rjmp	i2c_write_bit                             ; 2

i2c_get_ack:                                          ; 1
	sbi	SCL_DDR,SCL	;force SCL low
	cbi	SDA_DDR,SDA	;release SDA
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	cbi	SCL_DDR,SCL	;release SCL
i2c_ack_wait:
	sbis	SCL_IN,SCL	;wait SCL high (in case wait states are inserted)
	rjmp	i2c_ack_wait

	clr	r24		;return 0
	sbic	SDA_IN,SDA	;if SDA high -> return 1
	ldi	r24,1
    sbi	SCL_DDR,SCL	;force SCL low
	rcall	i2c_delay_T2	;delay T/2
	clr	r25
	ret
	.endfunc



;*************************************************************************
; read one byte from the I2C device, send ack or nak to device
; (ack=1, send ack, request more data from device 
;  ack=0, send nak, read is followed by a stop condition)
;
; extern unsigned char i2c_read(unsigned char ack);
;	ack = r24, return = r25(=0):r24
; extern unsigned char i2c_readAck(void);
; extern unsigned char i2c_readNak(void);
; 	return = r25(=0):r24
;*************************************************************************
	.global i2c_readAck
	.global i2c_readNak
	.global i2c_read		
	.func	i2c_read
i2c_readNak:
	clr	r24
	rjmp	i2c_read
i2c_readAck:
	ldi	r24,0x01
i2c_read:
	ldi	r23,0x01	;data = 0x01
i2c_read_bit:
	sbi	SCL_DDR,SCL	;force SCL low                     ; 2
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1
    nop                                                ; 1

	cbi	SCL_DDR,SCL	;release SCL                       ; 2

i2c_read_stretch:
    ;loop until SCL is high (allow slave to stretch SCL)
    sbis SCL_IN, SCL                                  ; 2
    rjmp	i2c_read_stretch
                                                      ; 
	clc			;clear carry flag                     ; 1
	sbic	SDA_IN,SDA	;if SDA is high               ; 2
	sec			;  set carry flag
                                                      ; 
	rol	r23		;store bit                            ; 1
    ;while receive register not full
	brcc	i2c_read_bit                              ; 2

i2c_put_ack:
	sbi	SCL_DDR,SCL	;force SCL low
	cpi	r24,1
	breq	i2c_put_ack_low	;if (ack=0)
	cbi	SDA_DDR,SDA	;      release SDA
	rjmp	i2c_put_ack_high
i2c_put_ack_low:                ;else
	sbi	SDA_DDR,SDA	;      force SDA low
    nop
i2c_put_ack_high:
	nop
	nop
	nop
	nop
	nop
	cbi	SCL_DDR,SCL	;release SCL
i2c_put_ack_wait:
	sbis	SCL_IN,SCL	;wait SCL high
	rjmp	i2c_put_ack_wait
	nop
	mov	r24,r23
	clr	r25
    sbi	SCL_DDR,SCL	;force SCL low
    cbi	SDA_DDR,SDA	;release SDA
	ret
	.endfunc

