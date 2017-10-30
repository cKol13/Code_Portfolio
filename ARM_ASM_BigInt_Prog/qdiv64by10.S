/*
    Author: Chris Kolegraff
    File: qdiv64by10.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function will return the quotient and the remainder 
    of 'b' / 10. This function implements the divison by multiplying by a 
    special constant, which represents the reciprocal of the divisor. 
    It returns the quotient in r1:r0, and the remainder in r2.
    
    unsigned long long qdiv64by10(unsigned long long top);
*/

    .text
    .global qdiv64by10
    
qdiv64by10:
    @ lo(top) in r0
    @ hi(top) in r1
    @ Quotient will be in r1:r0
    @ Remainder will be in r2
    stmfd sp!, {r4-r7, lr}  @ Push registers onto stack

    @ Load constant: 10 shifted right for 64 bits of precision + 1
    @ Store in r3:r2
    ldr r2, =0x66666667     
    ldr r3, =0x66666666     

    @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Get Quotient
    @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    mov r7, #0              @ Make sure r7:r4 is cleared
    mov r6, #0
    mov r5, #0
    mov r4, #0

    @ See if the upper word is empty, do shorter algorithm
    cmp r1, #0      
    beq .hiEmpty
    
    @ See if the lower word is empty, do shorter algorithm
    cmp r0, #0      
    beq .loEmpty
    
    @ Multiply r7:r4 = r1:r0 x r3:r2
    umull r4, r5, r2, r0    @ Multiply lo(const) and lo(top)
    umull r14, r12, r2, r1  @ Multiply lo(const) and hi(top)
    adds r5, r5, r14        @ Add lo(result) into r5, set flags
    adc  r6, r6, r12        @ Add hi(result) into r6
    umull r14, r12, r3, r0  @ Multiply hi(const) and lo(top)
    adds r5, r5, r14        @ Add lo(result) into r5, set flags
    adcs r6, r6, r12        @ Add hi(result) into r6, set flags
    addhs r7, r7, #1        @ Add 1 to r7 if carry bit set
    umull r14, r12, r3, r1  @ Multiply hi(const) and hi(top)
    adds r6, r6, r14        @ Add lo(result) into r6, set flags
    adc  r7, r7, r12        @ Add hi(result) into r7
    b .multDone
    
.hiEmpty:
    umull r4, r5, r2, r0    @ Multiply lo(const) and lo(top)
    umull r14, r12, r3, r0  @ Multiply hi(const) and lo(top)
    adds r5, r5, r14        @ Add hi(first) and lo(second)
    adc  r6, r6, r12        @ Add hi(second) with carry, store in r6
    b .multDone
    
    
.loEmpty:
    umull r5, r6, r2, r1    @ Multiply lo(const) and hi(top), set into r6:r5
    umull r14, r12, r3, r1  @ Multiply hi(const) and hi(top)
    adds r6, r6, r14        @ Add hi(first) and lo(second)
    adc  r7, r7, r12        @ Add hi(second) and carry
    
    @ Result held in r7:r4
    @ Disregard lower 2 words
    
.multDone:
    @ Shift r7:r6 right by 2 bits
    lsr r6, r6, #2          @ Shift low word right 2 bits
    orr r6, r6, r7, lsl #30 @ Extract last 2 bits of high word, orr into r6
    lsr r7, r7, #2          @ Logical shift right
    @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Quotient is r7:r6

    @ Get modulus by shifting and adding -> 10*quotient
    @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    lsl r5, r7, #3          @ Shift hi(quotient) left by 3 (*8)
    orr r5, r5, r6, lsr #29 @ Extract top 3 bits of lo(quotient), orr into r5
    lsl r4, r6, #3          @ Shift lo(quotient) left by 3
    
    lsl r3, r7, #1          @ Shift hi(quotient) left by 1 (*2)
    orr r3, r3, r6, lsr #31 @ Extract top bit of lo(quotient), orr into r3
    lsl r2, r6, #1          @ Shift lo(quotient) left by 3
    
    add r2, r2, r4          @ Add 8q + 2q, ignore carry and top word
    sub r2, r0, r2          @ Calculate the remainder
    @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    @ Result of modulus in r2

    mov r1, r7              @ Mov r7:r6 into r1:r0
    mov r0, r6

    ldmfd sp!, {r4-r7, pc}  @ Return Quotient, Remainder
