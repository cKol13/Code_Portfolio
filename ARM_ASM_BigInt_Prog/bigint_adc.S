/*
    Author: Dr. Pyeatt
    File: bigint_adc.S
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This function is meant to add together two bigints, and to 
    add in a carry as well. After adding the two bigints together, the function
    will check to make sure that the sum result is an appropriate size. It will
    trim away all leading 0 chunks in the blks array of the struct.
    This function does dynamically allocate and free storage.
    
    bigint bigint_adc(bigint l, bigint r, int carry);
*/

	.equ	bi_blks,0	@ offset to block pointer
	.equ	bi_size,4	@ offset to size int
	.equ	bi_struct_sz,8	@ size of the bigint struct

	.equ	NULL,0
	
@@@ bigint bigint_adc(bigint l, bigint r, int carry)
@@@ This function adds two big integers along with a carry bit.
@@@ NOTE: labels begining with "." are ignored by the profiler.
	.text
	.global bigint_adc
        .type   bigint_adc, %function
bigint_adc:
	stmfd	sp!,{r4-r11,lr}	@ store everything
	@@ They may have different lengths. Put longest on left.
	ldr	r8,[r0,#bi_size]
	ldr	r9,[r1,#bi_size]
	cmp	r8,r9
	bge	.noswap
	@@ skip next part if they are already ordered
	mov	r3,r0		@ swap pointers
	mov	r0,r1
	mov	r1,r3
	mov	r3,r8		@ swap sizes
	mov	r8,r9
	mov	r9,r3
.noswap:@@ r4  : pointer to blocks for longest bigint
	@@ r5  : pointer to blocks for shortest bigint
	@@ r6  : pointer to blocks for result
	@@ r7  : loop counter
	@@ r8  : size of longest bigint
	@@ r9  : size of shortest bigint
	@@ r10 : CPSR flags
	@@ r11 : tmp1
	@@ r12 : tmp2
	ldr	r4,[r0,#bi_blks]  @ load pointer
	lsl	r10,r2,#29        @ initialize carry bit
	ldr	r5,[r1,#bi_blks]  @ load pointer
	lsl	r0,r8,#2	  @ calcualte result size
	bl 	malloc		  @ allocate storage
	cmp	r0,#NULL	  @ check for NULL
	bleq	alloc_err
	mov	r6,r0
	mov	r7,#0		  @ initilize loop counter
.loopa: ldr	r11,[r4,r7, lsl #2]@ load current chunk
	ldr	r12,[r5,r7, lsl #2]@ load current chunk
	msr	CPSR_f,r10	  @ restore flags (carry bit)
	adcs	r11,r11,r12	  @ add chunks
	str	r11,[r6,r7, lsl #2]@ store result
	add	r7,r7,#1	  @ increment count
	mrs	r10,CPSR	  @ save flags in r10
	cmp	r7,r9		  @ cmp will change flags
	blt	.loopa
	@@ We may have chunks remaining in the longest bigint
	cmp	r7,r8		  @ are there any chunks remaining?
	bge	.finish
	tst	r12,#0x80000000	  @ fill r12 with the sign of
	moveq	r12,#0		  @ the shorter bigint
	movne	r12,#0xFFFFFFFF
.loopb:	ldr	r11,[r4,r7, lsl #2]@ load current chunk
	msr	CPSR_f,r10	  @ restore flags (carry bit)
	adcs	r11,r11,r12	  @ add chunks
	str	r11,[r6,r7, lsl #2]@ store result
	add	r7,r7,#1	  @ increment count
	mrs	r10,CPSR	  @ save flags in r10
	cmp	r7,r8		  @ cmp will change flags
	blt	.loopb
.finish:@@ if there was overflow on the final add, then
	@@ extend the result and copy the sign (carry) bit
	msr	CPSR_f,r10	  @ restore flags
	bvc	.noext
	movcc	r11,#0
	movcs	r11,#0xFFFFFFFF
	lsl	r0,r8,#2	  @ calculate number of bytes
	add	r0,r0,#4	  @ increase storage space
	bl	malloc
	cmp	r0,#NULL	  @ check for NULL
	bleq	alloc_err
	mov	r4,r0		  @ protect new pointer
	mov	r1,r6		  @ get pointer to source
	lsl	r2,r8,#2	  @ calculate number of bytes
	bl	memcpy		  @ copy the data
	mov	r0,r6		  @ free the old storage
	bl	free
	str	r11,[r4,r8,lsl#2] @ store extended bits
	add	r8,r8,#1	  @ calculate new size
	mov	r6,r4
	b	.return
.noext:	@@ if we did not extend, then we may need to trim
	@@ r4  : current
	@@ r5  : next
	@@ r6  : pointer to blocks for result
	@@ r7  : i
	@@ r8  : size of result
	@@ r9  : new size of result	
	subs	r7,r8,#1	  @ i = size - 1
	ble	.return		  @ can't be trimmed
	mov	r9,r8 		  @ newsize = size;
	ldr	r5,[r6,r7,lsl #2] @ load next
	tst	r5,#0x80000000
	bne	.nloop
.ploop:	mov	r4,r5		@ current = next
	subs	r7,r7,#1	@ decrement index
	blt	.trimit		@ done if it is <= 0
	ldr	r5,[r6,r7,lsl #2] @ load next chunk
	cmp	r4,#0x00000000	@ done if not leading zeros
	bne	.trimit
	tst	r5,#0x80000000  @ done if next sign bit is set
	bne	.trimit
	sub	r9,r9,#1	@ current can be trimmed
	b	.ploop
.nloop: mov	r4,r5		@ current = next
	subs	r7,r7,#1	@ decrement index
	blt	.trimit		@ done if it is <= 0
	ldr	r5,[r6,r7,lsl #2] @ load next chunk
	cmp	r4,#0xFFFFFFFF	@ done if not leading ones
	bne	.trimit
	tst	r5,#0x80000000  @ done if next sign bit not set
	beq	.trimit
	sub	r9,r9,#1	@ current can be trimmed
	b	.nloop
.trimit:cmp	r8,r9
	beq	.return
	lsl	r0,r9,#2
	bl	malloc
	cmp	r0,#NULL	  @ check for NULL
	bleq	alloc_err
	mov	r10,r0
	mov	r1,r6
	lsl	r2,r9,#2
	bl	memcpy
	mov	r0,r6
	bl	free
	mov	r6,r10
	mov	r8,r9
.return:mov	r0,#bi_struct_sz
	bl	malloc
	cmp	r0,#NULL	  @ check for NULL
	bleq	alloc_err
	str	r8,[r0,#bi_size]  @ store size
	str	r6,[r0,#bi_blks]  @ store pointer to blocks
	ldmfd	sp!,{r4-r11,pc}	  @ return	
       .size   	bigint_adc, .-bigint_adc
