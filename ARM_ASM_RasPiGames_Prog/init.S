/*
File: init.S
Author: Dr. Pyeatt, Chris Kolegraff
Description:
This file has functions that initialize gpio pins to certain states. It also
contains the function for an introductory sequence to the program.
*/  
    
    .include "Consts.S"

    .text
    
    @ This functiopn sets up the RasPi IO and configures pin in/outputs 
    @ and any needed pullups
/*
Function: initSystem
Author: Chris Kolegraff
Description:
Initializes the program. It sets up pin input/outputs, as well as setting
pull-up/down resistor states. Will first call IO_Init to initialize the RasPi
IO to communicate with peripherals.
*/
    .global initSystem
initSystem:
    stmfd sp!, {lr}         @ Push registers to stack
	bl	IO_init             @ map the devices into our address space
	
	@ Set the direction bits for the pins
	
	mov	r0,#R_LED           @ Setup red LED's output pin
	bl	gpio_dir_output
	mov	r0,#B_LED           @ Setup blue LED's output pin
	bl	gpio_dir_output
	mov	r0,#Y_LED           @ Setup yellow LED's output pin
	bl	gpio_dir_output
	mov	r0,#G_LED           @ Setup green LED's output pin
	bl	gpio_dir_output
	mov r0, #SOFT_PWM_PIN   @ Setup software PWM's output pin
	bl gpio_dir_output
	
	mov	r0,#R_BTN           @ Setup red button's input pin
	bl	gpio_dir_input
	mov	r0,#B_BTN           @ Setup blue button's input pin
	bl	gpio_dir_input
	mov	r0,#Y_BTN           @ Setup yellow button's input pin
	bl	gpio_dir_input
	mov	r0,#G_BTN           @ Setup green button's input pin
	bl	gpio_dir_input
	mov	r0,#RESET_BTN       @ Setup reset button's input pin
	bl	gpio_dir_input

	mov	r0,#R_BTN           @ Setup red button to pull up
	mov	r1,#PULL_UP
	bl	set_pud
	
	mov	r0,#B_BTN           @ Setup blue button to pull up
	mov	r1,#PULL_UP
	bl	set_pud

	mov	r0,#Y_BTN           @ Setup yellow button to pull up
	mov	r1,#PULL_UP
	bl	set_pud
	
	mov	r0,#G_BTN           @ Setup green button to pull up
	mov	r1,#PULL_UP
	bl	set_pud
	
	mov	r0,#RESET_BTN       @ Setup reset button to pull up
	mov	r1,#PULL_UP
	bl	set_pud
	ldmfd sp!, {pc}         @ Pop registers and return
	
	
/*
Function: gpio_dir_output
Author: Dr. Pyeatt
Description:
Sets the direction of a pin in r0 to output
*/
	.global gpio_dir_output
gpio_dir_output:
	stmfd	sp!,{lr}        @ Push registers to the stack
	@ Load the Base Address of the GPIO device
	ldr	r1,=gpiobase	    @ load pointer to the address of the gpio device
	ldr	r1,[r1]		        @ load address of the gpio device
	bl	pin_offset 	        @ get the offset in r3 and shift in r0
	add	r1,r1,r3	        @ get pointer to the FPFSEL register
	ldr	r3,[r1]		        @ read the FPFSEL register
	mov	r2,#7		        @ create bit mask for 3 bits we need to clear
	lsl	r2,r2,r0
	mvn	r2,r2
	and	r3,r2,r3	        @ clear the 3 bits
	mov	r2,#1		        @ create bit mask for the bit we need to set
	lsl	r2,r2,r0
	orr	r2,r2,r3	        @ set the 3 bits
	str	r2,[r1]		        @ write the FPFSEL register
	str	r2,[r1]		        @ write the FPFSEL register
	ldmfd	sp!,{pc}        @ Pop registers and return
	
    
/*
Function: set_pud
Author: Dr. Pyeatt
Description:
enable/disable the internal pullup/pulldown resistors
set_pud(gpio_pin_num, direction)
direction: 0=float, 1=pull down, 2=pull up
*/
set_pud:stmfd	sp!,{r0-r3}
	@ Load the Base Address of the GPIO device
	ldr	r3,=gpiobase	    @ load pointer to the address of the gpio device
	ldr	r3,[r3]		        @ load address of the gpio device
	@ Enable the internal pull-up resistors on button pins
 	@ Store direction in GPPUD
 	str	r1,[r3,#GPPUD]
 	mov	r2,#SHORT_DELAY
w1:	subs	r2,r2,#1
 	bcc	w1
 	@ Write to GPPUDCLK0 to clock the signals into GPIO 22,23,24
 	mov	r2,#1
	lsl	r2,r2,r0
 	str	r2,[r3,#GPPUDCLK0]
 	mov	r2,#SHORT_DELAY
w2:	subs	r2,r2,#1
 	bcc	w2
	@ Write to GPPUD to remove the control signal
 	mov	r2,#0
 	str	r2,[r3,#GPPUD]
 	@ Write to GPPUDCLK0/1 to remove the clock 
 	@ Write to GPPUDCLK0 to clock the signals into GPIO 22,23,24
 	mov	r2,#0
 	str	r2,[r3,#GPPUDCLK0]
	ldmfd	sp!,{r0-r3}
	mov	pc,lr

	
    
    
/*
Function: pin_offset
Author: Dr. Pyeatt
Description:
pin_offest finds the offset to the correct GPFSEL register
it returns the offset in r3, and the shift amount in r0
*/
pin_offset:	
	subs	r0,r0,#10	    @ pins 0-9 are in GPFSEL0
	movlt	r3,#GPFSEL0
	blt	have_offset
	subs	r0,r0,#10	    @ pins 10-19 are in GPFSEL1
	movlt	r3,#GPFSEL1
	blt	have_offset
	subs	r0,r0,#10	    @ pins 20-29 are in GPFSEL2
	movlt	r3,#GPFSEL2
	blt	have_offset
	subs	r0,r0,#10	    @ pins 30-39 are in GPFSEL3
	movlt	r3,#GPFSEL3
	blt	have_offset
	subs	r0,r0,#10	    @ pins 40-49 are in GPFSEL4
	movlt	r3,#GPFSEL4
	blt	have_offset
	sub	r0,r0,#10	        @ pins 40-49 are in GPFSEL5
	mov	r3,#GPFSEL5
have_offset:
	add	r0,r0,#10	        @ make 0<=r0<=9
	add	r0,r0,r0,asl #1     @ r0 *= 3

	mov	pc,lr

    
/*
Function: gpio_dir_input
Author: Dr. Pyeatt
Description:
gpio_dir_input sets the pin specified in r0 to be an input pin
*/
    .global gpio_dir_input
gpio_dir_input:
	stmfd	sp!,{lr}
	@ Load the Base Address of the GPIO device
	ldr	r1,=gpiobase	    @ load pointer to the address of the gpio device
	ldr	r1,[r1]		        @ load address of the gpio device
	bl	pin_offset 	        @ get the offset in r3 and shift in r0
	add	r1,r1,r3	        @ get pointer to the FPFSEL register
	ldr	r3,[r1]		        @ read the FPFSEL register
	mov	r2,#7		        @ create bit mask for 3 bits we need to clear
	lsl	r2,r2,r0
	mvn	r2,r2
	and	r2,r2,r3	        @ clear the 3 bits 
	str	r2,[r1]		        @ write the FPFSEL register
	str	r2,[r1]		        @ write the FPFSEL register
	ldmfd	sp!,{pc}
	
/*
Function: intro_sequence
Author: Chris Kolegraff
Description:
Plays a sequence of LEDs/tones at the beginning of program startup.
*/
	.global intro_sequence
intro_sequence:
    stmfd sp!, {r4, lr}         @ Push registers to the stack
    ldr r4, =gpiobase           @ Load address to gpiobase
	ldr r4, [r4]                @ Load the gpiobase address
	
	mov r0, r4                  @ Move gpiobase address into r0
    bl turn_off_LEDs            @ Make sure all LEDs are off
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #G_LED              @ Move green LED into r1
    mov r2, #E3_NOTE            @ Move E3 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #Y_LED              @ Move green LED into r1
    mov r2, #A3_NOTE            @ Move A3 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #R_LED              @ Move green LED into r1
    mov r2, #CS4_NOTE           @ Move CS4 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #B_LED              @ Move green LED into r1
    mov r2, #E4_NOTE            @ Move E4 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    ldmfd sp!, {r4, pc}         @ Pop registers and return
