/*
    Author: Chris Kolegraff
    File: bigint_mul.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will return the result of multiplying two 
    bigints together. bigint_mul uses the algorithm from Section 7.2.5
    The strategy for the most speed up I am using is to try and pull as much 
    dynamic memory allocation out of the loop as possible, and put it into 
    the highestlevel of function calls. Meaning, I want to allocate all dynamic
    memory in bigint_mul and have all the functions used by bigint_mul expect 
    those variables they use as temps. The function will allocate memory at
    the beginning of the function call, and no memory allocation or freeing
    will be performed in the multiplication algorithm's loop.
    
    bigint bigint_mul(bigint l, bigint r);
*/

    .text
    .global bigint_mul
    
    .extern bigint_alloc
    .extern bigint_mul_uint
    .extern bigint_shift_left_chunk
    .extern accumulate_noTrim
    .extern bigint_free
    
    .equ bi_blks, 0
    .equ bi_size, 4
    .equ bis_size, 8
    
bigint_mul:
    stmfd sp!, {r4-r11, lr} @ Push everything onto stack
    @ r0 is 'bigint l'
    @ r1 is 'bigint r'
    
    @ r4 is l
    @ r5 is r, then r->blks
    @ r6 is l->size, then i
    @ r7 is r->size
    @ r8 is sum
    @ r9 is tmp1
    @ r10 is tmp3
    @ r11 is shiftResult
    
    
    mov r4, r0              @ Save l into r4
    mov r5, r1              @ Save r into r5
    ldr r6, [r4, #bi_size]  @ Load l->size
    ldr r7, [r5, #bi_size]  @ Load r->size
    
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Allocate bigint variables
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    @ Allocate space for sum, used for storing mul result
    add r0, r6, r7          @ r0 = l->size + r->size
    bl bigint_alloc         @ Allocate bigint of size l->size+r->size
    mov r8, r0              @ Store sum into r8
    
    @ Allocate space for tmp1, used for storing uint_mul results
    add r0, r6, #1          @ r0 = l->size + 1
    bl bigint_alloc         @ Allocate bigint of size l->size+ 1
    mov r9, r0              @ Store tmp1 into r9
    
    @ Allocate space for tmp3, use as a temp variable for uint_mul
    mov r0, #3              @ Move 3 into r0
    bl bigint_alloc         @ Allocate bigint of size 3
    mov r10, r0             @ Store tmp3 into r10
    
    @ Allocate space for shiftResult, used for storing shift results
    ldr r0, [r9, #bi_size]  @ Load tmp1->size (l->size+1)
    add r0, r0, r7          @ Add r0 = l->size+1 + r->size
    bl bigint_alloc         @ Allocate bigint of size l->size + r->size + 1
    mov r11, r0             @ Store shiftResult into r11
    
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Initialize bigint variables
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Only sum and tmp1 need to be initialized to 0
    @ Initialize sum to 0
    mov r0, #0                  @ Use r0 as i
    ldr r1, [r8, #bi_size]      @ Load sum->size
    ldr r2, [r8]                @ Load sum->blks
    mov r3, #0                  @ Use to clear values
    
.sumInitLoop: 
    str r3, [r2, r0, lsl #2]    @ Set sum->blks[i] to 0
    add r0, r0, #1              @ Increment i by 1
    cmp r0, r1                  @ Compare i to sum->size
    blt .sumInitLoop
    
    @ Initialize tmp1 to 0
    mov r0, #0                  @ Use r0 as i
    ldr r1, [r9, #bi_size]      @ Load tmp1->size
    ldr r2, [r9]                @ Load tmp1->blks
.tmp1InitLoop: 
    str r3, [r2, r0, lsl #2]    @ Set tmp1->blks[i] to 0
    add r0, r0, #1              @ Increment i by 1
    cmp r0, r1                  @ Compare i to tmp1->size
    blt .tmp1InitLoop
    
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Perform the multiply algorithm
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    ldr r5, [r5]                @ Load r->blks into r5
    mov r6, #0                  @ Use r6 as i
    sub r7, r7, #1              @ Load r->size - 1 into r7

.multLoop:
    cmp r6, r7                  @ Compare i to r->size - 1
    bge .multDone
    @ Call bigint_mul_uint(l,r->blks[i], tmp1, tmp3, shiftResult);
    mov r0, r4                  @ Load l
    ldr r1, [r5, r6, lsl #2]    @ Load r->blks[i]
    mov r2, r9                  @ Load tmp1
    mov r3, r10                 @ Load tmp3
    str r11, [sp, #-4]!         @ Store shiftResult onto stack as 5th argument
    bl bigint_mul_uint          @ Call multiply function
    add sp, sp, #4              @ Deallocate space on stack
    
    @ Call bigint_shift_left_chunk(tmp1,i, shiftResult);
    mov r0, r9                  @ Load tmp1 into r0
    mov r1, r6                  @ Load i into r1
    mov r2, r11                 @ Load shiftResult into r2
    bl bigint_shift_left_chunk  @ Call shifting function
    
    @ Call accumulate_noTrim(sum, shiftResult);
    mov r0, r8                  @ Load sum into r0
    mov r1, r11                 @ Load shiftResult into r1
    bl accumulate_noTrim        @ Call accumulation function
    
    @ Reset tmp1 to all 0's
    mov r0, #0                  @ Use as j
    ldr r1, [r9]                @ Load tmp1->blks
    ldr r2, [r9, #bi_size]      @ Load tmp1->size
    mov r3, #0                  @ Use to clear words in tmp1
    
.tmp1ResetLoop:
    str r3, [r1, r0, lsl #2]    @ Store 0 into tmp1->blks[j]
    add r0, r0, #1              @ Increment j by 1
    cmp r0, r2                  @ Compare j to tmp1->size
    blt .tmp1ResetLoop
    
    add r6, r6, #1              @ Increment i by 1
    b .multLoop                 @ Loop back to top
    
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Perform last iteration of loop outside the for-loop,
    @ Call Dr. Pyeatt's bigint_adc to trim the result if needed.
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Call bigint_mul_uint(l,r->blks[i], tmp1, tmp3, shiftResult);
.multDone:
    mov r0, r4                  @ Load l
    ldr r1, [r5, r6, lsl #2]    @ Load r->blks[i]
    mov r2, r9                  @ Load tmp1
    mov r3, r10                 @ Load tmp3
    str r11, [sp, #-4]!         @ Store shiftResult onto stack as 5th argument
    bl bigint_mul_uint          @ Call multiply function
    add sp, sp, #4              @ Pop shiftResult off stack
    
    @ Call bigint_shift_left_chunk(tmp1,i, shiftResult);
    mov r0, r9                  @ Load tmp1 into r0
    mov r1, r6                  @ Load i into r1
    mov r2, r11                 @ Load shiftResult into r2
    bl bigint_shift_left_chunk  @ Call shifting function
    
    @ Call bigint_adc(sum,shiftResult,0); Save old sum to free later
    mov r4, r8                  @ Save sum into r4 to delete later
    mov r0, r8                  @ Move sum into r0
    mov r1, r11                 @ Move shiftResult into r1
    mov r2, #0                  @ Move 0 into r2
    bl bigint_adc               @ Do final bigint_adc with trimming
    mov r8, r0                  @ Save result back into sum
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Free bigints
    @ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    mov r0, r9                  @ Move tmp1 into r0
    bl bigint_free
    mov r0, r4                  @ Move tmp2 into r0
    bl bigint_free
    mov r0, r10                 @ Move tmp3 into r0
    bl bigint_free
    mov r0, r11                 @ Move shiftResult into r0
    bl bigint_free
    
    mov r0, r8                  @ Move sum into r0
    ldmfd sp!, {r4-r11, pc}     @ Pop everything, return sum
