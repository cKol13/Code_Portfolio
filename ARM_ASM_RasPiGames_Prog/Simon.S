/*
File: Simon.S
Author: Chris Kolegraff
Description:
This gamestate is for Simon Says. It will play the Simon Says game, then return
to the main game selection hub. 
*/
    
    .include "Consts.S"
    
    .equ MAX_NUMBERS, 300
    
    .data
    .align 2
sequence:           @ Array to hold the LED sequence for Simon Says, init to 0's
    .zero 1200
    .align 2
max_seq_length:     @ Used for storing highscore
    .word 0
    .align 2
    
fail_message:       @ Output a fail message, current score, and highscore
    .ascii "Sorry! That wasn't right! Sequence length: %d.\n"
    .asciz "Your longest sequence was: %d.\n"
    .align 2
    
LED_Array:          @ Array containing LED pin values
    .word B_LED
    .word R_LED
    .word Y_LED
    .word G_LED
    .word 0
    .align 2
    
note_Array:         @ Array containing musical note values
    .word E4_NOTE
    .word CS4_NOTE
    .word A3_NOTE
    .word E3_NOTE
    .word 0
    .align 2
    
    .text
    @ Simon Says gameplay state
    
/*
Function: Simon_Says_State
Author: Chris Kolegraff
Description:
Plays the Simon Says game. First a random LED will be chosen as the first LED.
Then the user must match the LED with the correct button input. A new random
LED will be appended to the sequence, and the user will have to mimic the
LED sequence. Each correct sequence input will increase your score, and your
highest score will be saved. At the end of the game the function will tell you
your current score and highest score in the terminal.
*/
    .global Simon_Says_State
Simon_Says_State:
    stmfd sp!, {r4-r9, lr}
    bl clock                    @ Get processor tick count, use to seed rand()
    bl srand                    @ Seed the RNG with current clock tick #
    
    ldr r7, =gpiobase           @ Load address to gpiobase into r7
    ldr r7, [r7]                @ Load gpiobase address into r7
    
    mov r4, #0                  @ r4 tracks how long sequence is
    ldr r5, =sequence           @ r5 holds the sequence string
    
    mov r0, r7                  @ Move gpiobase into r0
    bl turn_off_LEDs            @ Turn off all LEDs
    mov r0, #2                  @ 2 second delay
	ldr r1, =0                  
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    
    @1->blue, 2->red, 3->yellow, 4->green
loop:
    @ ADD NEW NUMBER TO SEQUENCE
    bl rand                     @ 
    and r0, r0, #3              @ Get last 2 bits, random number from 0 to 3
    str r0, [r5, r4, lsl #2]    @ Store new value into sequence[i]
    add r4, r4, #1              @ Increment r4 by 1
    
    
    
    @ DISPLAY SEQUENCE TO PLAYER
    mov r6, #0                  @ Use as temp index
.disp_loop:
    ldr r3, [r5, r6, lsl #2]    @ Load sequence, random number 0-3
    ldr r1, =LED_Array          @ Load LED Array, [Blue, Red, Yellow, Green]
    ldr r0, [r1, r3, lsl #2]    @ Get the Target LED from LED_Array[i]
    ldr r1, =note_Array         @ Load note Array, [E4, C#4, A3, E3]
    ldr r1, [r1, r3, lsl #2]    @ Get the Target note from note_Array[i]
    
    stmfd sp!, {r1}             @ Store note onto stack
    
    mov r1, r0                  @ Move LED into r1
    mov r0, r7                  @ Move gpiobase into r0
    bl turn_on_LED              @ Turn the target LED on
    
    
    
    
    ldmfd sp!, {r0}             @ Pop note back into r0
    bl play_note                @ Play the note
    mov r0, #0                  @ 0.75 second delay
    ldr r1, =750000             
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    bl stop_all_notes           @ Stop playing the note
    
    mov r0, r7                  @ Load gpiobase into r0
    bl turn_off_LEDs            @ Turn off all the LEDs
    mov r0, #0                  @ 0.25 second delay
	ldr r1, =250000
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    add r6, r6, #1              @ Increment temp index
    cmp r6, r4                  @ Compare temp index to current length of seq.
    blt .disp_loop              @ Branch to top if more LEDs in sequence
    
    
    
    @ USER INPUT LOOP
    @ r8 holds temp counter
    mov r8, #0                  @ Move 0 into r8
    
.inputLoop:
    bl blockingButtonRead       @ Get digital read of button, blocking input
    mov r9, r0                  @ Save result (0-3) into r9
    ldr r1, =LED_Array          @ Load LED Array address
    ldr r1, [r1, r0, lsl #2]    @ Get LED_Array[i]
    mov r0, r7                  @ Move gpiobase into r0
    bl turn_on_LED              @ Light up LED
    
    ldr r0, =note_Array         @ Load note Array address
    ldr r0, [r0, r9, lsl #2]    @ Get note_Array[i]
    bl play_note                @ Play the note
    mov r0, #0                  @ 0.75 second delay
    ldr r1, =750000
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    
    bl stop_all_notes           @ Stop playing all notes
    mov r0, r7                  @ Move gpiobase into r0
    bl turn_off_LEDs            @ Turn all LEDs off
    
    mov r0, r9                  @ Move button pressed into r0
    ldr r1, [r5, r8, lsl #2]    @ Load current sequence value
    
    cmp r0, r1                  @ Compare user input to sequence
    bne .fail                   @ If different, go to fail safe
    
    @ Correct input, check if there's more
    add r8, r8, #1              @ Increment temp index
    cmp r8, r4                  @ Compare temp index # to total #'s in sequence 
    blt .inputLoop              @ Read more LEDs if not done with sequence
    
    bl Success_State            @ Player successfully pressed all buttons
    
    mov r0, r7                  @ Move gpiobase into r0
    bl turn_off_LEDs            @ Make sure all LEDs are off
    mov r0, #0                  @ 300 millisecond delay
    ldr r1, =300000
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    
    cmp r4, #MAX_NUMBERS        @ Compare current seq. length to max length
    blt loop                    @ Branch back to top.
    @ Player shouldn't be so good as to remember 300 #s in a sequence
    @ Otherwise just fail them for being too good, or cheating, and return 
    
.fail:
    ldr r0, =RS_NOTE            @ Load the raspberry note
    bl play_note                @ Play the raspberry note
    mov r0, #1                  @ Delay for 1 second
    mov r1, #0
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    ldr r0, =RS_NOTE            @ Load the raspberry note
    bl stop_note                @ Stop playing the note
    
    ldr r0, =fail_message       @ Load the failure message
    sub r4, r4, #1              @ Fet the # of correct rounds of simon says
    mov r1, r4                  @ Move success count into r1
    ldr r2, =max_seq_length     @ Get highscore address
    ldr r3, [r2]                @ Get highscore value
    cmp r3, r4                  @ Compare highscore to current score
    movlt r3, r4                @ If current score higher, replace new highscore
    strlt r3, [r2]              @ Store new highscore
    mov r2, r3                  @ Move new score into r2
    bl printf                   @ Print failure message and scores
    
    ldmfd sp!, {r4-r9, pc}      @ Pop registers and return
    
blockingButtonRead:
    stmfd sp!, {r4-r7, lr}      @ Push registers to stack
    ldr r4, =gpiobase           @ Load address to gpiobase
    ldr r4, [r4]                @ Get gpiobase address
    
.blockLoop:
    mov r0, r4                  @ Move gpiobase address into r0
    mov r1, #B_BTN              @ Read blue button pin
    bl getDebouncedValue        @ Digital read pin, non-blocking
    cmp r0, #1                  @ See if button was pressed
    moveq r0, #0                @ Move 0 into r0 if button pressed
    ldmeqfd sp!, {r4-r7, pc}    @ Pop and return 0 if button pressed
    
    mov r0, r4                  @ Move gpiobase address into r0
    mov r1, #R_BTN              @ Read red button pin
    bl getDebouncedValue        @ Digital read pin, non-blocking
    cmp r0, #1                  @ See if button was pressed
    moveq r0, #1                @ Move 1 into r0 if button pressed
    ldmeqfd sp!, {r4-r7, pc}    @ Pop and return 1 if button pressed
    
    mov r0, r4                  @ Move gpiobase address into r0
    mov r1, #Y_BTN              @ Read yellow button pin
    bl getDebouncedValue        @ Digital read pin, non-blocking
    cmp r0, #1                  @ See if button was pressed
    moveq r0, #2                @ Move 1 into r0 if button pressed
    ldmeqfd sp!, {r4-r7, pc}    @ Pop and return 1 if button pressed
    
    mov r0, r4                  @ Move gpiobase address into r0
    mov r1, #G_BTN              @ Read green button pin
    bl getDebouncedValue        @ Digital read pin, non-blocking
    cmp r0, #1                  @ See if button was pressed
    moveq r0, #3                @ Move 1 into r0 if button pressed
    ldmeqfd sp!, {r4-r7, pc}    @ Pop and return 1 if button pressed
    
    mov r0, #0                  @ 5ms delay
    ldr r1, =5000
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    b .blockLoop                @ Always branch to top of loop to block
    
    mov r0, #10                 @ Return 10, but this should never be called
    ldmfd sp!, {r4-r7, pc}      @ Should never get here
    
    
    
