/*
File: RasPi_IO.S
Author: Dr. Pyeatt, Chris Kolegraff
Description:
File contains functions used to map addresses from the physical address space
to the program's virtual memory address space. Gives you easy access to
gpiobase, pwmbase, uart0base, and clkbase addresses.
*/

@@@ Raspberry Pi devices
@@@ -----------------------------------------------------------
@@@ This file provides a function "IO_init" that will
@@@ map some devices into the user program's memory
@@@ space.  Pointers to the devices are stored in
@@@ global variables, and the user program can then
@@@ use those pointers to access the device registers.
@@@ -----------------------------------------------------------
	.data
@@@ -----------------------------------------------------------
@@@ The following global variables will hold the addresses of
@@@ the devices that can be accessed directly after IO_init
@@@ has been called.
	.global	gpiobase
gpiobase:	.word	0
	.global	pwmbase
pwmbase	:	.word	0
	.global	uart0base
uart0base:	.word	0
	.global	clkbase
clkbase	:	.word	0

@@@ These are the addresses for the I/O devices (after
@@@ the firmware boot code has remapped them).

@@@ ONE OF THE FOLLOWING TWO LINES SHOULD BE COMMENTED OUT, DEPENDING
@@@ ON WHETHER YOU HAVE AN ORIGINAL RASPBERRY PI,
@@@ OR A RASPBERRY PI 2
@@@ 	.equ	PERI_BASE,  0x20000000 @ start of all devices (RasPi)
 	.equ	PERI_BASE,  0x3F000000 @ start of all devices (RasPi2)

	@@ Base Physical Address of the GPIO registers
	.equ	GPIO_BASE, (PERI_BASE + 0x200000)
	@@ Base Physical Address of the PWM registers
	.equ	PWM_BASE,  (PERI_BASE + 0x20C000)
	@@ Base Physical Address of the UART 0 device
	.equ	UART0_BASE,(PERI_BASE + 0x201000)
	@@ Base Physical Address of the Clock/timer registers
	.equ	CLK_BASE,  (PERI_BASE + 0x101000)

	.equ	MAP_FAILED,-1
	.equ	MAP_SHARED, 1
	.equ	PROT_READ,  1
	.equ	PROT_WRITE, 2
	.equ 	BLOCK_SIZE,(4*1024)

	@@ some constants from fcntl.h
	.equ	O_RDONLY,   00000000
	.equ	O_WRONLY,   00000001
	.equ	O_RDWR,	    00000002
	.equ	O_CREAT,    00000100
	.equ	O_EXCL,	    00000200
	.equ	O_NOCTTY,   00000400
	.equ	O_TRUNC,    00001000
	.equ	O_APPEND,   00002000
	.equ	O_NONBLOCK, 00004000
	.equ	O_NDELAY,   O_NONBLOCK
	.equ	O_SYNC,     00010000
	.equ	O_FSYNC,    O_SYNC
	.equ	O_ASYNC,    00020000

memdev:		.asciz	"/dev/mem"
@ I have changed successstr for the RasPi Gaming Center program
successstr:	.ascii	"Welcome to the Raspi Gaming Center!\n"
                    .asciz "Initializing the Console...\n"

mappedstr:	.asciz	"Mapped %s device at 0x%08X\n"
openfailed:	.asciz	"IO_init: failed to open /dev/mem: "
mapfailedmsg:	.asciz	"IO_init: mmap of %s failed: "
gpiostr:	.asciz	"GPIO"
pwmstr:		.asciz	"PWM"
uart0str:	.asciz	"UART0"
clkstr:		.asciz	"CLK"	


	.text
	
/*
Function: IO_init
Author: Dr. Pyeatt, Chris Kolegraff
Description:
This function maps devices into memory space and stores their addresses
in global variables.

Error messaging has been commented out, only message that this function prints
is related to the RasPi Gaming Console program (Program 2).
*/
	.global	IO_init
IO_init:
	stmfd	sp!,{r4,r5,lr}
	@@ Try to open /dev/mem	
	ldr	r0,=memdev	@ load address of "/dev/mem"
	ldr	r1,=(O_RDWR + O_SYNC) @ set up flags
	bl	open		@ call the open syscall
	cmp	r0,#0		@ check result
	bge	init_opened	@ if open failed,
	ldr	r0,=openfailed	@ print message and exit 
	bl	printf
	bl      __errno_location
        ldr     r0, [r0]
        bl      strerror
        bl      perror
	mov	r0,#0		@ return 0 for failure
	b	init_exit
init_opened:
	@@ Open succeeded. Now map the devices 
	mov	r4,r0		@ move file descriptor to r4
	ldr	r0,=successstr
	bl	printf
	@@ Map the GPIO device
	mov	r0,r4		@ move file descriptor to r4
	ldr	r1,=GPIO_BASE   @ address of device in memory
	bl	trymap
	cmp	r0,#MAP_FAILED
	ldrne	r1,=gpiobase	@ if succeeded, load pointer
	strne	r0,[r1]		@ if succeeded,	store value
	ldreq	r1,=gpiostr	@ if failed, load pointer to string
	beq	map_failed_exit	@ if failed, print message
	mov	r2,r1
	ldr	r2,[r2]
	/*
	ldr	r0,=mappedstr   @ print success message
	ldr	r1,=gpiostr
	bl	printf
	*/
	@@ Map the PWM device
	mov	r0,r4		@ move file descriptor to r4
	ldr	r1,=PWM_BASE	@ address of device in memory
	bl	trymap
	cmp	r0,#MAP_FAILED
	ldrne	r1,=pwmbase	@ if succeeded, load pointer
	strne	r0,[r1]		@ if succeeded,	store value
	ldreq	r1,=pwmstr	@ if failed, load pointer to string
	beq	map_failed_exit	@ if failed, print message
	mov	r2,r1
	ldr	r2,[r2]
	/*
	ldr	r0,=mappedstr   @ print success message
	ldr	r1,=pwmstr
	bl	printf
	*/
	@@ Map the UART0 device
	mov	r0,r4		@ move file descriptor to r4
	ldr	r1,=UART0_BASE	@ address of device in memory
	bl	trymap
	cmp	r0,#MAP_FAILED
	ldrne	r1,=uart0base	@ if succeeded, load pointer
	strne	r0,[r1]		@ if succeeded,	store value
	ldreq	r1,=uart0str	@ if failed, load pointer to string
	beq	map_failed_exit	@ if failed, print message
	mov	r2,r1
	ldr	r2,[r2]
	/*
	ldr	r0,=mappedstr   @ print success message
	ldr	r1,=uart0str
	bl	printf
	*/
	@@ Map the clock manager device
	mov	r0,r4		@ move file descriptor to r4
	ldr	r1,=CLK_BASE	@ address of device in memory
	bl	trymap
	cmp	r0,#MAP_FAILED
	ldrne	r1,=clkbase	@ if succeeded, load pointer
	strne	r0,[r1]		@ if succeeded,	store value
	ldreq	r1,=clkstr	@ if failed, load pointer to string
	beq	map_failed_exit	@ if failed, print message
	mov	r2,r1
	ldr	r2,[r2]
	/*
	ldr	r0,=mappedstr   @ print success message
	ldr	r1,=clkstr
	bl	printf
	*/
	@@ All mmaps have succeded.
	@@ Close file and return 1 for success
	mov	r5,#1
	b	init_close
map_failed_exit:
	@@ At least one mmap failed. Print error,
	@@ unmap everthing and return
	ldr	r0,=mapfailedmsg
	bl	printf
	bl      __errno_location
	ldr     r0, [r0, #0]
	bl      strerror
	bl      perror
	bl	IO_close
	mov	r0,#0
init_close:
	mov	r0,r4		@ close /dev/mem
	bl	close
init_exit:	
	ldmfd	sp!,{r4,r5,pc}	@ return
	

/*
Function: trymap
Author: Dr. Pyeatt
Description:
trymap(int fd, unsigned offset) Calls mmap.
*/
trymap: stmfd   sp!,{r5-r7,lr}
        mov     r5,r1           @ copy address to r5
        mov     r7,#0xFF        @ set up a mask for aligning
        orr     r7,#0xF00
        and     r6,r5,r7        @ get offset from page boundary
        bic     r1,r5,r7        @ align phys addr to page boundary
        stmfd   sp!,{r0,r1}     @ push last two params for mmap
        mov     r0,#0           @ let kernel choose virt address
        mov     r1,#BLOCK_SIZE
        mov     r2,#(PROT_READ + PROT_WRITE)
        mov     r3,#MAP_SHARED
        bl      mmap
        add     sp,sp,#8        @ pop params from stack
        cmp     r0,#-1
        addne   r0,r0,r6        @ add offset from page boundary
        ldmfd   sp!,{r5-r7,pc}
        
/*
Function: IO_close
Author: Dr. Pyeatt
Description:
IO_close unmaps all of the devices.
*/
	.global	IO_close
IO_close:
	stmfd	sp!,{r4,r5,lr}
	ldr	r4,=gpiobase	@ get address of first pointer
	mov	r5,#4		@ there ar 4 pointers
IO_closeloop:
	ldr	r0,[r4]		@ load address of device
	mov	r1,#BLOCK_SIZE
	cmp	r0,#0
	blgt	munmap		@ unmap it
	mov	r0,#0
	str	r0,[r4],#4	@ store and increment
	subs	r5,r5,#1
	bgt	IO_closeloop
	ldmfd	sp!,{r4,r5,pc}
