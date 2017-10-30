/*
    Author: Chris Kolegraff
    File: bigint_mul_uint.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will multiply a bigint by an unsigned int and 
    return a result of a bigint of size l->size+1. The result will be in the 
    sum variable passed in, as well as the return value. This function assumes 
    that 'sum' is a bigint of size l->size+1, and that it has been initialized 
    to all 0's. This function also assumes 'tmp1' is a bigint of size 3 that 
    has also been initialized to all 0's. It is also assumed that 'shiftResult' 
    is a bigint of size l->size+r->size+1 which does not need to be initlialized 
    to any value before being passed in. The function also assumes that 'l' and
    'r' are valid bigints.
    
    bigint bigint_mul_uint(bigint l, unsigned r, bigint sum, 
        bigint tmp1, bigint shiftResult);
*/
    
    .text
    .global bigint_mul_uint
    .extern bigint_shift_left_chunk
    .extern accumulate_noTrim
    
    .equ bi_blks, 0
    .equ bi_size, 4
    .equ bis_size, 8
    
bigint_mul_uint:
    mov r12, sp                 @ Save stack pointer
    stmfd sp!, {r4-r10, lr}     @ Push registers onto stack
    @ r0 is 'bigint l'
    @ r1 is 'unsigned r'
    @ r2 is 'bigint sum'
    @ r3 is 'bigint tmp1'
    
    @ r4 is l->blks
    @ r5 is r
    @ r6 is sum
    @ r7 is tmp1
    @ r8 is i
    @ r9 is l->size
    @ r10 is shiftResult
    
    ldr r10, [r12]              @ Load shiftResult
    mov r5, r1                  @ Save 'r' into r5
    mov r6, r2                  @ Save 'sum' into r6
    ldr r9, [r0, #bi_size]      @ Load l->size
    ldr r4, [r0]                @ Load l->blks array
    mov r7, r3                  @ Save tmp1 in r7
    mov r8, #0                  @ Use r8 as i
    
.uintLoop:
    ldr r2, [r4, r8, lsl #2]    @ Load l->blks[i]
    umull r0, r1, r2, r5        @ r1:r0 = l->blks[i] * r
    
    ldr r3, [r7]                @ Load tmp1->blks
    str r0, [r3]                @ Store r0 into tmp1->blks[0]
    str r1, [r3, #4]            @ Store r1 into tmp1->blks[1]
    mov r1, #0                  @ Move 0 into r1
    str r1, [r3, #8]            @ Store 0 into tmp1->blks[2]
    
    mov r0, r7                  @ Move tmp1 into r0
    mov r1, r8                  @ Move i into r1
    mov r2, r10                 @ Move shiftResult into r2
    bl bigint_shift_left_chunk  @ Shift tmp1 left i chunks
    
    mov r0, r6                  @ Move sum into r0
    mov r1, r10                 @ Move shiftResult into r1
    bl accumulate_noTrim        @ Add together sum and tmp1
    
    add r8, r8, #1              @ Increment i by 1
    cmp r8, r9                  @ Comapre i to l->size
    blt .uintLoop
    
    mov r0, r6                  @ Move sum into r0
    ldmfd sp!, {r4-r10, pc}     @ Pop and return sum;
