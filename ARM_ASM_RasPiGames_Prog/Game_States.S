/*
File: Game_States.S
Author: Chris Kolegraff
Description:
Contains functions that lead to different game states, including an introduction
sequence, a freeplay mode, and a success state.
*/

.include "Consts.S"
    .data
    .align 2
    
    @ Random sequence of LEDs for the Introduction State
IS_Sequence:
    .word B_LED
    .word Y_LED
    .word R_LED
    .word G_LED
    .word R_LED
    .word B_LED
    .word Y_LED
    .word G_LED
    .word B_LED
    .word R_LED
    .word 0
    .align 2
             
    @ Random sequence of LEDs for the Success State       
Success_Sequence:
    .word R_LED
    .word B_LED
    .word Y_LED
    .word R_LED
    .word G_LED
    .word B_LED
    .word Y_LED
    .word R_LED
    .word B_LED
    .word G_LED
    .word 0
    .align 2
    
    .text
    .extern tickerInit
    .extern wait
/*
Function: Intro_State
Author: Chris Kolegraff
Description:
Intro_State runs before each game, as well as when entering central game hub
It will rapidly flash all 4 LEDs in a quick pattern, then a slower pattern
During the slower pattern, it will play a tone depending on which LED is lit.
*/
    .global Intro_State
Intro_State:
    stmfd sp!, {r4-r9, lr}      @ Pop registers to the stack
    ldr r4, =gpiobase           @ Load address to gpiobase value
    ldr r4, [r4]                @ Load gpiobase address
    
    mov r6, #0                  @ Use r6 as index to sequence array
    ldr r7, =IS_Sequence        @ Load sequence array
    
    mov r0, r4                  @ Move gpiobase address into r0
    bl turn_off_LEDs            @ Make sure LEDs enter the sequence off
	mov r9, #0                  @ Use to count # of outside loop iterations
    
.loop:
    ldr r8, [r7, r6, lsl #2]    @ Load next LED in sequence
    cmp r8, #0                  @ See if we reached end of sequence
    moveq r6, #0                @ Reset index to 0
    beq .loop                   @ Reset loop, read the first value in sequence
    addne r6, r6, #1            @ Increment index by 1
    
    mov r0, r4                  @ Move gpiobase address into r0
    mov r1, r8                  @ Move current LED in sequence into r1
    bl turn_on_LED              @ Turn on the corresponding LED
    
    @ Add a delay to sequence
    mov r0, #0                  @ 50ms delay
    ldr r1, =50000
    bl tickerInit               @ Set delay amount
    bl wait                     @ Call delay function
    
    @ Update counter, loop if needed
	ldr r12, =IS_LOOPCOUNT      @ Load # of times to run the sequence loop
	add r9, r9, #1              @ Increment counter by 1
	cmp r9, r12                 @ See if we have more iterations to run
	blt .loop                   @ Loop if needed
    
    mov r0, r4                  @ Move gpiobase address into r0
    bl turn_off_LEDs            @ Make sure LEDs are cleared
	
    mov r0, #1                  @ 1 second delay
    mov r1, #0
    bl tickerInit               @ Set delay amount
    bl wait                     @ Call delay function
    
    @ Light up 4 LED's one at a time, B, R, Y, G, and play corresponding tones
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #B_LED              @ Move green LED into r1
    mov r2, #E4_NOTE            @ Move E3 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #R_LED              @ Move green LED into r1
    mov r2, #CS4_NOTE           @ Move E3 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #Y_LED              @ Move green LED into r1
    mov r2, #A3_NOTE            @ Move E3 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #G_LED              @ Move green LED into r1
    mov r2, #E3_NOTE            @ Move E3 note into r1
    bl light_with_delay_sound   @ Turn on light, play note, and delay
    
    mov r0, r4                  @ Move gpiobase address into r0
    bl turn_off_LEDs            @ Make sure all LED's are off
    ldmfd sp!, {r4-r9, pc}      @ Pop and return
    
    
/*
Function: Freeplay_State
Author: Chris Kolegraff
Description:
A Gamestate of the program that will read button presses and then light up
an LED and play a tone depending on which button was pressed
*/
    .global Freeplay_State
Freeplay_State:
    stmfd sp!, {r4-r7, lr}      @ Push registers onto stack
    ldr r4, =gpiobase           @ Load address pointing to gpiobase
    ldr r4, [r4]                @ Load gpiobase address
    
freeplay_loop:
    mov r0, r4                  @ Move gpiobase into r0
    bl turn_off_LEDs            @ Make sure LED's are turned off
    
    mov r0, r4                  @ Move gpiobase into r0 
    mov r1, #B_LED              @ Move blue LED into r1
    mov r2, #B_BTN              @ Move blue BTN into r2
    mov r3, #E4_NOTE            @ Load E4 note into r3
    bl freeplay_button_test     @ Check if btn pressed, LED on/play note if so
    cmp r0, #1                  @ Check if button was pressed
    beq freeplay_loop           @ Button was pressed, branch back to top
    
    mov r0, r4                  @ Move gpiobase into r0 
    mov r1, #R_LED              @ Move red LED into r1
    mov r2, #R_BTN              @ Move red BTN into r2
    mov r3, #CS4_NOTE           @ Load C#4 note into r3
    bl freeplay_button_test     @ Check if btn pressed, LED on/play note if so
    cmp r0, #1                  @ Check if button was pressed
    beq freeplay_loop           @ Button was pressed, branch back to top
    
    mov r0, r4                  @ Move gpiobase into r0 
    mov r1, #Y_LED              @ Move yellow LED into r1
    mov r2, #Y_BTN              @ Move yellow BTN into r2
    mov r3, #A3_NOTE            @ Load A3 note into r3
    bl freeplay_button_test     @ Check if btn pressed, LED on/play note if so
    cmp r0, #1                  @ Check if button was pressed
    beq freeplay_loop           @ Button was pressed, branch back to top
    
    mov r0, r4                  @ Move gpiobase into r0 
    mov r1, #G_LED              @ Move green LED into r1
    mov r2, #G_BTN              @ Move green BTN into r2
    mov r3, #E3_NOTE            @ Load E3 note into r3
    bl freeplay_button_test     @ Check if btn pressed, LED on/play note if so
    cmp r0, #1                  @ Check if button was pressed
    beq freeplay_loop           @ Button was pressed, branch back to top
    
    mov r0, r4                  @ Move gpiobase into r0 
    mov r1, #RESET_BTN          @ Move reset button into r1
    bl getDebouncedValue        @ Non-blocking digital read of a button
    cmp r0, #1                  @ See if button was pressed
    bne freeplay_loop           @ Exit if button is pressed
    mov r0, r4                  @ Move gpiobase into r0
    bl turn_off_LEDs            @ Make sure LEDs are turned off
    @ Exit if reset button pushed
    ldmfd sp!, {r4-r7, pc}      @ Pop registers, return

/*
Function: freeplay_button_test
Author: Chris Kolegraff
Description:
This function reads a button, and if that button is pressed, it will light
up its corresponding LED and play the given note as well.
*/
freeplay_button_test:
    @ r0 has gpiobase
    @ r1 has LED
    @ r2 has BTN
    @ r3 has note
    stmfd sp!, {r4-r7, lr}      @ Push registers onto stack
    mov r4, r0                  @ Save gpiobase into r4
    mov r5, r1                  @ Save LED into r5
    mov r6, r2                  @ Save BTN into r6
    mov r7, r3                  @ Save note into r7
    @ r0 still has gpiobase
    bl turn_off_LEDs            @ Make sure LED's are turned off
    
    mov r0, r4                  @ Move gpiobase into r0 
    mov r1, r6                  @ Move button into r1
    bl getDebouncedValue        @ Non-blocking digital read of a button
    cmp r0, #1                  @ See if the button was pressed
    movne r0, #0                @ Make sure r0 has 0 in it if btn not pressed
    bne .fp_b_t_exit            @ Exit if not pressed
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, r5                  @ Move LED into r1
    bl turn_on_LED              @ Turn on the LED
    mov r0, r7                  @ Move note into r0
    bl play_note                @ Start playing
    mov r0, #0                  @ Delay for 100ms
    ldr r1, =100000
    bl tickerInit               @ Set delay amount
    bl wait                     @ Call delay function
    mov r0, r7                  @ Load note into r0
    bl stop_note                @ Stop playing
    @ Return whether the button was pressed, 1 for pressed, 0 for not
    mov r0, #1                  
.fp_b_t_exit:
    ldmfd sp!, {r4-r7, pc}      @ Pop registers and return

/*
Function: Success_State
Author: Chris Kolegraff
Description:
Will rapidly flash 4 LEDs in quick succession, and play a small fanfare.
*/
    .global Success_State
Success_State:
    stmfd sp!, {r4-r9, lr}      @ Push registers to stack
    ldr r4, =gpiobase           @ Load address to gpiobase
    ldr r4, [r4]                @ Load gpiobase address
    ldr r5, =SUCCESS_LOOPCOUNT  @ Load # of times to run success loop
    ldr r6, =Success_Sequence   @ Load sequence of LED's to light up
    mov r7, #0                  @ Track how many loop iterations there are
    mov r8, #0                  @ Use to index success sequence
success_loop:
    ldr r9, [r6, r8, lsl #2]    @ Load next LED
    cmp r9, #0                  @ See if end of sequence
    moveq r8, #0                @ Reset index to 0
    addeq r7, r7, #1            @ Increment loop count
    beq success_loop            @ Reset loop, load beginning of sequence
    addne r8, r8, #1            @Increment index
    
    mov r0, r4                  @ Load gpiobase into r0
    mov r1, r9                  @ Move chosen LED into r1
    bl turn_on_LED              @ Turn the corresponding LED
    
    cmp r9, #B_LED              @ If blue LED, play E4
    ldreq r0, =E4_NOTE
    cmp r9, #R_LED              @ If red LED, play C#4
    ldreq r0, =CS4_NOTE
    cmp r9, #Y_LED              @ If yellow LED, play A3
    ldreq r0, =A3_NOTE
    cmp r9, #G_LED              @ If green LED, play E3
    ldreq r0, =E3_NOTE
    
    bl play_note                @ Play the chosen tone
    
    mov r0, #0                  @ Wait 0.x seconds
    ldr r1, =SUCCESS_DELAY      
    bl tickerInit               @ Set delay amount
    bl wait                     @ Call delay function
    
    bl stop_all_notes           @ Stop playing any notes
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, r9                  @ Move LED into r1
    bl turn_off_LED             @ Turn off LED
    
    cmp r7, r5                  @ Check to see if we have more iterations
    blt success_loop            @ Branch to top if so
    ldmfd sp!, {r4-r9, pc}      @ Pop registers and return

