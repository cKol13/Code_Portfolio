/*
    Author: Chris Kolegraff
    File: bigint_smallmod.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will return the quotient, and the remainder, 
    of 'b' / 'num'. This function will either use a multiply-by-constant-
    reciprocal or a long division method to calculate the remainder. 
    If 'num' is 10, then it will use the constant method, as this bigint struct
    also supports outputting the bigint to the terminal, which requires a 
    divison by 10 anyway. The function does no memory allocation or freeing. 
    This function stores the quotient of 'b'/'num' into 'b'. The remainder is
    returned to the user.
    
    unsigned bigint_smallmod(bigint b,chunk num);
*/

    .text
    .global bigint_smallmod
    .extern udiv32
    .extern udiv64
    
    .equ bi_blks, 0
    .equ bi_size, 4
    .equ bis_size, 8

bigint_smallmod:
    stmfd sp!, {r4-r6, lr}
    @ r0 is 'bigint b' 
    @ r1 is 'chunk num'
    ldr r2, [r0, #bi_size]      @ Load b->size
    ldr r6, [r0]                @ Load b->blks array
    mov r5, r1                  @ Save num in r5
    @ Use r12 and r7 as bigchunk tmp
    mov r12, #0
    
    sub r2, r2, #1              @ Decrement b->size by 1
    ldr r1, [r6, r2, lsl #2]    @ Load b->blks[b->size-1]
    mov r3, r1                  @ Put result in lo(tmp)
    mov r4, r2                  @ Use r4 as i, load with b->size - 1
    
    @ Use slower algorithm if the user is not modding by 10
    cmp r5, #10                 @ Compare num to 10 
    bne .smallLoop2 

.smallLoop1:    
    cmp r4, #0                  @ Compare i to 0
    ble .done1                  @ Branch if i <= 0
    
    mov r1, r12                 @ Move hi(tmp) into r1
    mov r0, r3                  @ Move lo(tmp) into r0
    bl qdiv64by10               @ Multiply by reciprocal to divide by 10
    
    @ r1:r0 is quotient, r2 is remainder

    str r0, [r6, r4, lsl #2]    @ Store lo(quotient) into b->blks[i]
    mov r12, r2                 @ Move the remainder into hi(tmp)
    mov r2, r4                  @ Move i into r2
    sub r2, r2, #1              @ Decrement i by 1
    ldr r2, [r6, r2, lsl #2]    @ Load b->blks[i-1]
    mov r3, r2                  @ Move next chunk in array to lo(tmp)
    sub r4, r4, #1              @ Decrement i by 1
    b .smallLoop1               @ Loop back to top
    
.smallLoop2:
    cmp r4, #0                  @ Compare i to 0
    ble .done2                  @ Branch if i <= 0

    mov r1, r12                 @ Move hi(tmp) into r1
    mov r0, r3                  @ Move lo(tmp) into r0

    mov r3, #0                  @ Move 0 into r3
    mov r2, r5                  @ Move num into r2
    bl udiv64                   @ Do a divide, quot in r1:r0, mod in r3:r2
    str r0, [r6, r4, lsl #2]    @ Store lo(quotient) into b->blks[i]
    mov r12, r2                 @ Move the lo(modulus) into hi(tmp)
    mov r2, r4                  @ Move i into r2
    sub r2, r2, #1              @ Decrement i by 1
    ldr r2, [r6, r2, lsl #2]    @ Load b->blks[i-1]
    mov r3, r2                  @ Move next chunk in array to lo(tmp)
    sub r4, r4, #1              @ Decrement i by 1
    b .smallLoop2               @ Loop back to top

.done1:
    mov r1, r12                 @ Move hi(tmp) into r1
    mov r0, r3                  @ Move lo(tmp) into r0
    
    bl qdiv64by10               @ Do a divide, quot in r1:r0, mod in r2
    str r0, [r6]                @ Store lo(quotient) into b->blks[0]
    mov r0, r2                  @ Move modulus into r0 to return
    ldmfd sp!, {r4-r6, pc}      @ return modulous
    
.done2:
    mov r1, r12                 @ Move hi(tmp) into r1
    mov r0, r3                  @ Move lo(tmp) into r0
    
    mov r3, #0                  @ Move 0 into r3
    mov r2, r5                  @ Move num into r2
    bl udiv64                   @ Do a divide, quot in r1:r0, mod in r3:r2
    str r0, [r6]                @ Store lo(quotient) into b->blks[0]
    mov r0, r2                  @ Move lo(modulus) into r0 to return
    ldmfd sp!, {r4-r6, pc}      @ return tmp
