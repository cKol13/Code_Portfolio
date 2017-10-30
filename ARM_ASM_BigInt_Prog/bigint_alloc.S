/*
    Author: Chris Kolegraff
    File: bigint_alloc.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will allocate space for a bigint with a blks 
    array of size chunks*4. The size field will be initialized to chunks, but 
    the blks array will not be initialized to a particular value. This does no 
    checking to ensure that space has been allocated. It will be up to the 
    user to check for these errors, as the struct has been optimized for speed. 
    
    bigint bigint_alloc(int chunks);
*/
    
    .text
    .global bigint_alloc
    
    .equ bi_blks, 0
    .equ bi_size, 4
    .equ bis_size, 8
    
bigint_alloc:
    stmfd sp!, {r4-r5, lr}  @ Push registers to the stack
    mov r4, r0              @ Save chunks in r4
    mov r0, #8              @ Move 8 into r0
    bl malloc               @ Call malloc(8)
    
    mov r5, r0              @ Save r in r5
    str r4, [r5, #bi_size]  @ Store chunks in r->size
    lsl r0, r4, #2          @ Load chunks << 2 into r0
    bl malloc               @ Malloc space for r->blks
    str r0, [r5]            @ Store array into r->blks
    
    mov r0, r5              @ Move r into r0
    ldmfd sp!, {r4-r5, pc}  @ Pop registers and return r
