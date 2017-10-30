/*
    Author: Chris Kolegraff
    File: bigint_from_int.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will allocate space for a bigint of size 1. 
    The word in the blks array will be initlialized to val. 
    
    bigint bigint_from_int(int val);
*/
    
    .text
    .global bigint_from_int
    .extern bigint_alloc
    
bigint_from_int:
    stmfd sp!, {r0, lr}     @ Store val onto stack
    mov r0, #1              @ Move 1 into r0
    bl bigint_alloc         @ Call bigint_alloc(1)
    ldmfd sp!, {r2, lr}     @ Load val into r2, get lr back
    
    ldr r1, [r0]            @ Load tmp->blks
    str r2, [r1]            @ Store val into tmp->blks[0]
    mov pc, lr              @ Return tmp
