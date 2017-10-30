/*
    Author: Larry Pyeatt
    File: divide_Functions.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: These functions are only used for the bigint_smallmod 
    function in the case where the user doesn't want to divide the input by 10.
    More complete descriptions of these functions can be found below.
    
    unsigned udiv32(unsigned dividend, unsigned divisor);
    unsigned long long udiv64(unsigned long long dividend, 
        unsigned long long divisor);
*/
    
    .text
    .global	udiv32
    .global	udiv64
    
@@@ -----------------------------------------------------------------
	@@ udiv32 takes a 32-bit unsigned dividend in r0 and
	@@ divides it by a 32-bit unsigned divisor in r1.
	@@ Returns the quotient in r0 and remainder in r1
	@@ It calls no other functions and only 
        @@ uses r0-r3. We don't need to use the stack
	
udiv32:	cmp	r1,#0		@ if divisor == zero
	beq	quitudiv32	@   exit immediately
	mov     r2,r1           @ move divisor to r2
        mov     r1,r0           @ move dividend to r1
        mov     r0,#0           @ clear r0 to accumulate result
        mov     r3,#1           @ set "current" bit in r3
divstrt:cmp	r2,#0           @ WHILE ((msb of r2 != 1)
	blt	divloop
	cmp     r2,r1		@ && (r2 < r1))
        lslls   r2,r2,#1        @  shift dividend left
        lslls   r3,r3,#1        @  shift "current" bit left
        bls     divstrt		@ end WHILE
divloop:cmp     r1,r2           @ if dividend >= divisor
        subhs   r1,r1,r2        @   subtract divisor from dividend
        addhs   r0,r0,r3        @   set "current" bit in the result
        lsr	r2,r2,#1	@ shift dividend right
        lsrs    r3,r3,#1	@ Shift current bit right into carry
        bcc     divloop         @ If carry not clear, R3 has shifted
                                @ one bit past where it started, and we
                                @ are done.
quitudiv32:	
        mov     pc,lr
    
@@@ -----------------------------------------------------------------
	@@ udiv64 takes a 64 bit unsigned dividend in r1:r0
	@@ and divides it by a 64 bit unsigned divisor in r3:r2
        @@ Returns a 64-bit result in r1:r0 and
	@@ 64-bit modulus in r3:r2
	
udiv64:
	@@ check for divisor of zero
	cmp	r2,#0
	cmpeq	r3,#0
	beq	quitudiv64
	stmfd	sp!,{r4-r6}
	mov	r4,r2		@ move divisor to r5:r4
	mov	r5,r3
	mov	r2,r0		@ move dividend to r3:r2
	mov	r3,r1
        mov     r0,#0           @ clear r1:r0 to accumulate result
        mov     r1,#0
        mov     r6,#0           @ set counter to zero
divstrt64:
	@@ shift divisor left until its msb is set, or
	@@   until divisor>=dividend
	@@ WHILE (( divisor > 0) && (divisor < dividend))
	cmp	r5,#0		@ is msb of divisor set?
	blt	divloop64	@ end loop if msb of divisor is set
	cmp     r5,r3           @ compare high words
	cmpeq	r4,r2		@ if they are equal, compare low words
	bhs	divloop64	@ end loop if divisor >= dividend (unsigned!)
	lsl	r5,r5,#1	@   shift divisor left(high 32 bits)
	lsls	r4,r4,#1	@   shift divisor left(low 32 bits)
	orrcs	r5,r5,#1	@   shift divisor left(carry low to high)
        add	r6,r6,#1	@   increment count
        b     	divstrt64	@ end WHILE

divloop64:
	lsl	r1,r1,#1	@ shift quotient left (high 32 bits)
	lsls	r0,r0,#1	@ shift quotient left (low 32 bits)
	orrcs	r1,r1,#1	@ shift quotient left (carry from low to high)
	
	cmp     r5,r3           @ compare divisor to dividend
	cmpeq	r4,r2		@ if they are equal, compare low words
	bhi	NoSub		@ IF (divisor<=dividend) Unsigned!
	subs	r2,r2,r4	@   subtract divisor from dividend
	sbc	r3,r3,r5	
	orr	r0,r0,#1	@   set lsb of quotient
	
NoSub:	lsr	r4,r4,#1	@ shift divisor right (low 32 bits)
	lsrs	r5,r5,#1	@ shift divisor right (high 32 bits)
	orrcs	r4,r4,#(1<<31)  @ shift divisor right (carry from high to low)
	subs	r6,#1		@ decrement count
	bge	divloop64	@ continue until count is negative

        ldmfd	sp!,{r4-r6}
quitudiv64:	
        mov     pc,lr
