/*
    Author: Chris Kolegraff
    File: accumulate_noTrim.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function is meant to add together two bigints, and to 
    store the result back into the left bigint. l = l + r. No memory 
    allocation or freeing is done in this function. It assumes l and r are 
    valid bigints and that l->size >= r->size. The function will return l.
    
    bigint accumulate_noTrim(bigint l, bigint r);
*/

    .text
    
    .equ bi_blks, 0
    .equ bi_size, 4
    .equ bis_size, 8
    
    .global accumulate_noTrim
    
@ Sum together l = l + r
accumulate_noTrim:
    stmfd sp!, {r4, lr}         @ Push registers onto stack
    ldr r2, [r1, #bi_size]      @ Load r->size
    ldr r1, [r1]                @ Load r->blks
    ldr r12, [r0]               @ Load l->blks
    mov r4, #0                  @ Use r4 as i
    
.accLoop:
    ldr r3, [r1, r4, lsl #2]    @ Load value at r->blks[i]
    ldr r14, [r12, r4, lsl #2]  @ Load value at l->blks[i]
    add r14, r14, r3            @ Add l->blks[i] + r->blks[i]
    str r14, [r12, r4, lsl #2]  @ Store value at l->blks[i]
    add r4, r4, #1              @ Increment i by 1
    cmp r4, r2                  @ Compare i to r->size
    blt .accLoop                @ Loop back if i < r->size
    
    ldmfd sp!, {r4, pc}         @ return l
