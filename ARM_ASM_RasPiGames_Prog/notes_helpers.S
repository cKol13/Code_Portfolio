/*
File: notes_helpers.S
Author: Chris Kolegraff
Description:
File contains assembly functions so that the ticker.c file has easy access to
the software PWM pins and can toggle them on/off.
*/
    
    .include "Consts.S"

    
    .text
/*
Function: buzzer_on
Author: Chris Kolegraff
Description:
turns the software PWM pin to 'on' state, called from C
*/
    .global buzzer_on
buzzer_on:
    stmfd sp!, {lr}         @ Push registers to stack
    ldr r0, =gpiobase       @ Load address to gpiobase
    ldr r0, [r0]            @ Load gpiobase address
    mov r1, #SOFT_PWM_PIN   @ Load software PWM pin into r1
    bl turn_off_buzzPin     @ Turn pin low, (pin is active low)
    ldmfd sp!, {pc}         @ Pop registers and return
    
/*
Function: buzzer_off
Author: Chris Kolegraff
Description:
Turns the software PWM pin to 'off' state, called from C
*/
    .global buzzer_off
buzzer_off:
    stmfd sp!, {lr}         @ Push registers to stack
    ldr r0, =gpiobase       @ Load address to gpiobase
    ldr r0, [r0]            @ Load gpiobase address
    mov r1, #SOFT_PWM_PIN   @ Load software PWM pin into r1
    bl turn_on_buzzPin      @ Turn pin high, (pin is active low)
    ldmfd sp!, {pc}         @ Pop registers and return
    
