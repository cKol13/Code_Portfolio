    @ Define which gpio pins to use
    .equ B_LED, 17
    .equ R_LED, 27
    .equ Y_LED, 22
    .equ G_LED, 23
    .equ SOFT_PWM_PIN, 12
    .equ B_BTN, 5
    .equ R_BTN, 6
    .equ Y_BTN, 13
    .equ G_BTN, 26
    .equ RESET_BTN, 19
    
    @ Pull up or pull down resister consts, used for pud
    .equ PULL_DOWN, 1
    .equ PULL_UP, 2
	.equ SHORT_DELAY, 50
	
	 @ These are the offsets for the GPIO registers
	.equ	GPFSEL0,   0x0000
	.equ	GPFSEL1,   0x0004
	.equ	GPFSEL2,   0x0008
	.equ	GPFSEL3,   0x000C
	.equ	GPFSEL4,   0x0010
	.equ	GPFSEL5,   0x0014
	.equ	GPSET0,    0x001C
	.equ	GPSET1,    0x0020
	.equ	GPCLR0,    0x0028	
	.equ	GPCLR1,    0x002c	
	.equ	GPLEV0,    0x0034

	.equ	GPPUD,     0x0094
	.equ	GPPUDCLK0, 0x0098
 	.equ	GPPUDCLK1, 0x009C 
 	
 	@ Introduction Sequence loopcount
 	.equ IS_LOOPCOUNT, 30
 	
 	@ Success State values
 	.equ SUCCESS_LOOPCOUNT, 3
 	.equ SUCCESS_DELAY, 25000 @ 25 ms delay
 	
 	@ Notes to play, E4, C#4, A3, E3, and raspberry sound
 	.equ E4_NOTE, 0
 	.equ CS4_NOTE, 1
 	.equ A3_NOTE, 2
 	.equ E3_NOTE, 3
 	.equ RS_NOTE, 4
 	
 	@ Starting delay values for LED Lineup gameplay, can't be 1,000,000 or more
 	.equ LINEUP_BTN_DELAY, 999999 @ 1 second
 	.equ LINEUP_LED_DELAY, 999999 @ 1 second
