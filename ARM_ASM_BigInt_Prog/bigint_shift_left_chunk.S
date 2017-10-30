/*
    Author: Chris Kolegraff
    File: bigint_shift_left_chunk.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will shift left the words stored in l->blks by
    'chunks' amount and store the result in 'result' as well as return 
    'result'. The function assumes that 'l' and 'result' are valid bigints, 
    and that 'result' is a bigint of size l->size+chunks at least.
    The algorithm will also set any words beyond l->size to 0, so you do not 
    need to initialize 'result' before calling this function.
    
    bigint bigint_shift_left_chunk(bigint l, int chunks, bigint result);
*/

    .text
    .global bigint_shift_left_chunk
    
    .equ bi_blks, 0
    .equ bi_size, 4
    .equ biz_size, 8
    
bigint_shift_left_chunk:
    stmfd sp!, {r4-r6, lr}      @ Store registers to stack
    @ r0 = 'l', then l->blks
    @ r1 = 'chunks'
    @ r2 = 'result'
    @ r3 = l->size
    @ r4 = i
    @ r5 = j
    @ r6 = temp
    @ r12 = result->size
    @ r14 = result->blks
    
    ldr r3, [r0, #bi_size]      @ Load l->size
    ldr r0, [r0]                @ Load l->blks
    ldr r12, [r2, #bi_size]     @ Load result->size
    ldr r14, [r2]               @ Load result->blks
    mov r4, #0                  @ Move 0 into r4, use as i
    mov r5, #0                  @ Move 0 into r5, use as j
    
.shiftLoop:
    cmp r4, r1                  @ Compare i to chunks
    bge .next1                  @ If i >= chunks, skip
    mov r6, #0                  @ Clear r6 to store
    str r6, [r14, r4, lsl #2]   @ Store 0 into result->blks[i]
    b .loopDone

.next1:
    cmp r5, r3                  @ Compare j to l->size
    bge .next2                  @ If j >= l->size, skip
    ldr r6, [r0, r5, lsl #2]    @ Load l->blks[j]
    add r5, r5, #1              @ Increment j by 1
    str r6, [r14, r4, lsl #2]   @ Store l->blks[j] into result->blks[i]
    b .loopDone
    
.next2:
    mov r6, #0                  @ Move 0 into r6 to clear
    str r6, [r14, r4, lsl #2]   @ Store 0 into result->blks[i]
    
    
.loopDone:
    add r4, r4, #1              @ Increment i by 1
    cmp r4, r12                 @ Compare i and result->size
    blt .shiftLoop              @ If i < result->size, loop back
    
    mov r0, r2                  @ Move result into r0, return result
    ldmfd sp!, {r4-r6, pc}      @ return result;
