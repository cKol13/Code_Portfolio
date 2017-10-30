/*
File: LEDpinControl.S
Author: Chris Kolegraff
Description:
This file includes functions that are used to read and write to various
GPIO pins. 
*/
    
    .include "Consts.S"

    .text
    
/*
Function: getDebouncedValue
Author: Chris Kolegraff
Description:
Performs a non-blocking digital read of an input pin in r0. The pin should be
configured to be an input, preferally with pull-up/down selected.
*/
    .global getDebouncedValue
getDebouncedValue:
    @ r0 has gpiobase
    @ r1 has pin value
    stmfd sp!, {r4-r6, lr}
    mov r4, r0                  @ Save gpiobase address
    mov r5, r1                  @ Save pin to read
    mov r0, #0                  @ delay for 0.5ms
    ldr r1, =500
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    ldr r0, [r4, #GPLEV0]       @ Load pins 0-31's input values
    mov r2, #1                  @ Move 1 into r2 to extract pin's data
    lsl r2, r2, r5              @ Shift 1 left by gpio pin #
    ands r2, r2, r0             @ Extract digital value of pin
    movne r0, #0                @ Move 0 into r0 if bit set
    moveq r0, #1                @ Move 1 into r0 if bit clear
    
    ldmfd sp!, {r4-r6, pc}      @ Pop registers and return
    
/*
Function: turn_off_LEDs
Author: Chris Kolegraff
Description:
Turns off all LEDs by setting all the output ports high.
The RasPi is active low, so outputting a 1 sends an off signal.
*/
    .global turn_off_LEDs
turn_off_LEDs:
    @ r0 has gpiobase address
    ldr r1, =0xFFFFFFFF         @ Load r1 with mask of all 1s
	str r1,[r0,#GPSET0]         @ Clear all pins
	mov pc, lr                  @ Return 
	
/*
Function: turn_off_LED
Author: Chris Kolegraff
Description:
Turn off 1 specific LED.
*/
    .global turn_off_LED
turn_off_LED:
    @ gpiobase in r0
    @ LED in r1
    mov r3, #1                  @ Load 1 into r3, for clearing pin
    lsl r3, r3, r1              @ Shift 1 left by LED amount
    str r3, [r0, #GPCLR0]       @ Set gpclr0's contents
    mov pc, lr                  @ Return
    
/*
Function: turn_on_LED
Author: Chris Kolegraff
Description:
Turn on 1 specific LED.
*/
    .global turn_on_LED
turn_on_LED:
    @ gpiobase in r0
    @ LED in r1
    stmfd sp!, {r4-r5, lr}      @ Push registers to stack
    mov r4, r0                  @ Save gpiobase into r4
    mov r2, #1                  @ Move 1 into r2
    lsl r1, r2, r1              @ Shift 1 left by LED amount
    ldr r5, =0xFFFFFFFF         @ Bit mask of all 1s
    bic r5, r5, r1              @ Bit clear bit we want to set
    str	r5,[r4,#GPSET0]	        @ Set pin state
	mvn r5, r5                  @ Flip bits of r5
	str	r5, [r4,#GPCLR0]        @ Clear some pins
	
    ldmfd sp!, {r4-r5, pc}      @ Pop registers and return
    
/*
Function: turn_on_buzzPin
Author: Chris Kolegraff
Description:
Configures the software PWM pin to the high state.
*/
    .global turn_on_buzzPin
turn_on_buzzPin:
    @ r0 has gpiobase
    mov r2, #1                  @ Move 1 into r2
    lsl r2, r2, #SOFT_PWM_PIN   @ Shift 1 to the left to get soft PWM pin
    str	r2, [r0,#GPSET0]	    @ Set PWM pin high
	mov pc, lr                  @ Return
	
/*
Function: turn_off_buzzPin
Author: Chris Kolegraff
Description:
Configres the software PWM pin to the low state.
*/
	.global turn_off_buzzPin
turn_off_buzzPin:
    @ r0 has gpiobase
    mov r2, #1                  @ Move 1 into r2
    lsl r2, r2, #SOFT_PWM_PIN   @ Shift 1 to the left to get soft PWM pin
    str	r2, [r0,#GPCLR0]	    @ Clear PWM pin to low
	mov pc, lr                  @ Return
	
/*
Function: light_with_delay_sound
Author: Chris Kolegraff
Description:
Will turn on an LED, and start playing a given sound. It will then wait for
1 second before turn the sound and LED off.
*/
    .global light_with_delay_sound
light_with_delay_sound:
    @ r0 has gpio
    @ r1 has LED
    @ r2 has note
    stmfd sp!, {r4, lr}      @ Push registers to stack
    mov r4, r0                  @ Save gpiobase into r4
    mov r5, r2                  @ Save note into r5
    
    @ r0 still has gpiobase in it
    @ r1 still has LED in it
    bl turn_on_LED              @ Turn the LED on
    mov r0, r5                  @ Move note into r0
    bl play_note                @ Start playing the note
    
	mov r0, #1                  @ 1 second delay
	mov r1, #0
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    
    bl stop_all_notes               @ Stop playing notes
    mov r0, r4                        @ Move gpiobase into gpiobase
    bl turn_off_LEDs            @ Turn the LEDs off
    ldmfd sp!, {r4, pc}      @ Pop registers and return
    
