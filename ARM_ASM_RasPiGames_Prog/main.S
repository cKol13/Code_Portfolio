/*
File: main.S
Author: Chris Kolegraff
Description:
This file holds the main function. It also includes strings to provide extra
guidance to the user concerning which state the game console is in currently.
*/
    
    .include "Consts.S"
    
 	.data
    .align 2
intOut:
    .asciz "Value: %d\n"
    .align 2
    
introduction:
    .ascii "Entering the game selection hub.\n"
    .ascii "Blue button plays Simon Says.\nRed button enters freeplay mode.\n"
    .asciz "Yellow button plays LED Lineup.\n"
    .align 2
    
entering_freeplay:
    .asciz "Entering Freeplay Mode!\n"
    .align 2
freeplay_instructions:
    .ascii "Welcome to Freeplay Mode! Press the colored buttons to make "
    .ascii "the LEDs light up and to play a tone!\nPress the blank button to "
    .asciz "return to the game selection hub.\n"
    .align 2
exiting_freeplay:
    .asciz "Exiting Freeplay Mode. Returning to the game selection hub.\n"
    .align 2
    
entering_SS:
    .asciz "Playing Simon Says!\n"
    .align 2
SS_instructions:
    .ascii "Welcome to Simon Says! Match the pattern "
    .asciz "and break your highscores!\n"
    .align 2
exiting_SS:
    .asciz "Exiting Simon Says. Returning to the game selection hub.\n"
    .align 2
    
entering_lineup:
    .asciz "Playing LED Lineup!\n"
    .align 2
lineup_instructions:
    .ascii "Welcome to LED Lineup! Press the corresponding button as fast "
    .ascii "as you can! Time limit gets increasingly smaller the more correct "
    .asciz "answers you give.\n"
    .align 2
exiting_lineup:
    .asciz "Exiting LED Lineup. Returning to the game selection hub.\n"
    .align 2
    
	.text
/*
Function: main
Author: Chris Kolegraff
Description:
This function has the main game loop. It will initialize the game console, then
it will play an introductory sequence to let the user know it has initialized.
Next, it will enter the game loop. A different intro sequence will play, then
the user will be able to select which game mode they want to enter.
Blue -> Simon Says
Red -> Freeplay
Yellow -> LED Lineup

The program will loop endlessly until the program is shut down with Ctrl+C or
if the Raspberry Pi is shut down.
*/
	.global	main
main:	stmfd	sp!,{r4-r5,lr}
	bl initSystem               @ Init pins, pud's, etc
	bl intro_sequence           @ Play an introduction sequence
	ldr r4, =gpiobase           @ Load address to gpiobase
	ldr r4, [r4]                @ Load gpiobase address
	
    @ Program will have 3 different game states
    @ Game1 is original Simon Says
    @ Game2 is freeplay / keyboard
    @ Game3 is LED Lineup / "Dance Dance Revolution"
    
loop:
    bl Intro_State              @ Play an intro state each iteration of gameplay
    ldr r0, =introduction       @ Load intro string
    bl printf                   @ Print introducton to user
    @ Decide which game mode player wants
.loop2:
    @ Blue button, Simon Says mode
    mov r0, r4                  @ Move gpiobase address into r0
    mov r1, # B_BTN             @ Move blue button value into r1
    bl getDebouncedValue        @ Get button state, non-blocking digital read
    cmp r0, #1                  @ See if button was pressed
    bne .next1                  @ If not, move on
    ldr r0, =entering_SS        @ Load Simon Says intro
    bl printf                   @ Print to user
    bl Intro_State              @ Play introduction sequence
    ldr r0, =SS_instructions    @ Load Simon Says instructions
    bl printf                   @ Print instructions
    bl Simon_Says_State         @ Enter Simon Says gameplay state
    ldr r0, =exiting_SS         @ Load Simon Says exit message
    bl printf                   @ Print exit message
    b loop                      @ Loop back to top
    
.next1:
    @ Red button, Freeplay Mode
    mov r0, r4                      @ Move gpiobase address into r0
    mov r1, # R_BTN                 @ Move blue button value into r1
    bl getDebouncedValue            @ Get btn state, non-blocking digital read
    cmp r0, #1                      @ See if button was pressed
    bne .next2                      @ If not, move on
    ldr r0, =entering_freeplay      @ Load freeplay intro
    bl printf                       @ Print to user
    bl Intro_State                  @ Play introduction sequence
    ldr r0, =freeplay_instructions  @ Load freeplay instructions
    bl printf                       @ Print instructions
    bl Freeplay_State               @ Enter freeplay gameplay state
    ldr r0, =exiting_freeplay       @ Load freeplay exit message
    bl printf                       @ Print exit message
    b loop                          @ Loop back to top
    
.next2:    
    @ Yellow Button, LED Lineup
    mov r0, r4                      @ Move gpiobase address into r0
    mov r1, # Y_BTN                 @ Move blue button value into r1
    bl getDebouncedValue            @ Get btn state, non-blocking digital read
    cmp r0, #1                      @ See if button was pressed
    bne .loopDone                   @ If not, move on
    ldr r0, =entering_lineup        @ Load Lineup intro
    bl printf                       @ Print to user
    bl Intro_State                  @ Play introduction sequence
    ldr r0, =lineup_instructions    @ Load Lineup instructions
    bl printf                       @ Print instructions
    bl LEDLineup                    @ Enter Lineup gameplay state
    ldr r0, =exiting_lineup         @ Load Lineup exit message
    bl printf                       @ Print exit message
    b loop                          @ Loop back to top
    
.loopDone:
    mov r0, #0                      @ 5ms delay
    ldr r1, =5000
    bl tickerInit                   @ Setup delay function
    bl wait                         @ Call delay function
    b .loop2                        @ Loop back to intro and button reading loop
    
    
    
	ldmfd	sp!,{r4-r5,pc}          @ Shouldn't ever get here
	
    
  
