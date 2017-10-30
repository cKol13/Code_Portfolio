/*
    Author: Chris Kolegraff
    File: bigint_free.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will free a bigint structure of its allocated 
    memory. This function does not check if the bigint or the blks array are 
    NULL first, so it is up to the user to check for valid pointers 
    before freeing.
    
    void bigint_free(bigint b);
*/
    
    .text
    .global bigint_free
    
bigint_free:
    stmfd sp!, {r4, lr}
    mov r4, r0          @ Move bigint b into r4
    ldr r0, [r4]        @ Load b->blks into r0
    bl free             @ Free b->blks array
    mov r0, r4          @ Move b address into r0
    bl free             @ Free b struct
    ldmfd sp!, {r4, pc}
