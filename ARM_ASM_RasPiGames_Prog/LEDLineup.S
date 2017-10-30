/*
File: LEDLineup.S
Author: Chris Kolegraff
Description:
This file holds the function LEDLineup. This game is selected by pressing the
yellow button.

Function: LEDLineup
This game state plays the LED Lineup game. Press and hold the corresponding
button until the next button lights up to score a point. Get as many points
as you can as the game gets faster. The game will fail if multiple buttons 
are held down.
*/
    
    
    .include "Consts.S"
    
    .data
    .align 2
outputStr: @ String to output user when they fail, outputs score and highscore
    .ascii "You didn't press the LED in time! Your score was: %d!\n"
    .asciz "Your highscore is: %d!\n"
    .align 2
highscore: @ Tracks this game's highscore
    .word 0
    .align 2
LED_Array: @ Store LED's in an array
    .word B_LED
    .word R_LED
    .word Y_LED
    .word G_LED
    .word 0
    .align 2
Note_Array: @ Store Notes in an array
    .word E4_NOTE
    .word CS4_NOTE
    .word A3_NOTE
    .word E3_NOTE
    .word 0
BTN_Array: @ Store Buttons in an array
    .word B_BTN
    .word R_BTN
    .word Y_BTN
    .word G_BTN
    .word 0
    .align 2
    
    .text
    

    .global LEDLineup
LEDLineup:
    stmfd sp!, {r4-r10, lr}     @ Push registers onto stack
    mov r0, #1                  @ Delay for 1s
    mov r1, #0
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    
    bl clock                    @ Get processor tick count, use to seed rand()
    bl srand                    @ Seed RNG
    
    ldr r4, =gpiobase           @ Load address to gpiobase
    ldr r4, [r4]                @ Load gpiobase address
    
    ldr r6, =LINEUP_BTN_DELAY   @ starting time to delay for btn presses
    ldr r7, =LINEUP_LED_DELAY   @ starting time to delay LED/tone
    mov r8, #0                  @ Used to count score
    
.gameLoop:
    bl rand                     @ Choose random LED
    mov r5, r0                  @ Save result in r5
    and r5, r5, #3              @ Get random number between 0-3
    
    mov r0, r4                  @ Move gpiobase into r0
    ldr r1, =LED_Array          @ Load LED Array
    ldr r1, [r1, r5, lsl #2]    @ Load selected LED
    bl turn_on_LED              @ Turn the selected LED on
    ldr r0, =Note_Array         @ Load Note Array
    ldr r0, [r0, r5, lsl #2]    @ Load selected Note
    bl play_note                @ Start playing the note
    
    mov r0, #0                  @ Delay for LINEUP_LED_DELAY(r7)ms
    mov r1, r7                  @ Move LED delay into r1
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    
    mov r0, r4                  @ Move gpiobase into r0
    bl turn_off_LEDs            @ Turn off all LEDs
    bl stop_all_notes           @ Stop playing all notes
    
    
    @ Wait for user input with timeout
    mov r0, #0
    mov r1, r6                  @ Move button delay value into r1
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    
    
    @ Read all buttons, sum the results together.
    @ This will prevent multiple buttons being pressed at once
    mov r10, #0                 @ Use r10 to sum button results
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #B_BTN              @ Move blue button into r1
    bl getDebouncedValue        @ Read button value (0 or 1)
    add r10, r10, r0            @ Sum result with r10
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #R_BTN              @ Move red button into r1
    bl getDebouncedValue        @ Read button value (0 or 1)
    add r10, r10, r0            @ Sum result with r10
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #Y_BTN              @ Move yellow button into r1
    bl getDebouncedValue        @ Read button value (0 or 1)
    add r10, r10, r0            @ Sum result with r10
    
    mov r0, r4                  @ Move gpiobase into r0
    mov r1, #G_BTN              @ Move green button into r1
    bl getDebouncedValue        @ Read button value (0 or 1)
    add r10, r10, r0            @ Sum result with r10
    
    @ Get state of target's button
    ldr r1, =BTN_Array          @ Load Button Array
    ldr r1, [r1, r5, lsl #2]    @ Load selected Button
    mov r0, r4                  @ Move gpiobase into r0
    bl getDebouncedValue        @ Read button value (0 or 1)
    
    cmp r0, #1                  @ See if button was pressed
    bne .fail                   @ Fail if target button not pressed
    cmp r10, #1                 @ Make sure only one button was pressed
    bgt .fail                   @ Fail if too many btns pressed (2+ will fail)
    
    b .pass                     @ Go to passing portion of code
    
.fail:
    @ If failed, update highscore, output statistics, exit
    ldr r0, =outputStr          @ Load output format string
    mov r1, r8                  @ Load current score into r1
    ldr r2, =highscore          @ Load address of highscore
    ldr r2, [r2]                @ Load highscore into r2
    bl printf                   @ Print results/exit prompt
    b .quit                     @ Exit the game state
    
    @ If passed, decrease timeout value, update score, loop
.pass:
    add r8, r8, #1              @ Increment score by 1
    
    @ Update score
    ldr r0, =highscore          @ Load address of highscore
    ldr r1, [r0]                @ Load value of highscore
    cmp r8, r1                  @ Compare current score to highscore
    strgt r8, [r0]              @ Store current into HS, if current > highscore
    
    
    ldr r1, =20000              @ Load 20000 into r1
    sub r6, r6, r1              @ Reduce button delay by 20ms
    sub r7, r7, r1              @ Reduce LED delay by 20ms
    
    mov r0, #0                  @ Delay for 5ms
    ldr r1, =5000
    bl tickerInit               @ Setup delay function
    bl wait                     @ Call delay function
    b .gameLoop                 @ Branch back to top of game loop
    
.quit:
    ldmfd sp!, {r4-r10, pc}     @ Pop registers and return
